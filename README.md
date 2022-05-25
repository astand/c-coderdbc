  # CODERDBC 
  
  ## What is it

  CLI utilty for generating C code from dbc (CAN matrix) files

  ## Build and run

  This manual works on Ubuntu 20.04 and Windows 10. You need to ensure that your system has
  C++ compile and builing toolchain (**c++17**)

  To build coderdbc you need to make next steps:
  
  1 install cmake
  
  2 download source code:

  `git clone https://github.com/astand/c-coderdbc.git coderdbc`
  
  3 goto source code directory:

  `cd coderdbc`

  4 run cmake configuration to 'build' directory:

  `cmake -S src -B build`

  5 run cmake build:

  `cmake --build build --config release`

  6 goto to build directory and run:

  `cd build`

  `./coderdbc`

  Call without argument will print instruction how to pass DBC file for generation

  ## Driver functionality description

  The full pack of source code (both generated and manually edited) will be looked this
  (presuming that the dbc driver name is "ecudb"):
      
     ecudb.c / ecudb.h                             (1) RO / lib

    Main driver which has all dbc frames structs / pack functions / unpack functions these 
    source files preferably to place in lib level directory because they are RO using model
    and can be 
    shared among few separated projects.

      ecudb-fmon.h                                 (2) RO / lib

    Contains monitoring functions signatures which can be optionally called from unpack frame. 
    Best option to place file beside Main driver files (1).

      ecudb-fmon.c                                 (3) app

    User defined functions with diagnostic purpose. DLC, rolling, checksum errors can be handled 
    automatically if specific configuration enabled. This file is user level source code.

      ecudb-config.h                               (4) app / inc*

    This is application specific configuration file. If you have a few projects (applications) 
    which referenced on single main driver (1,2) then each project have to have own copy of this 
    configuration. Source code (1,2) includes this configuration. If a few dbc matrix is in use 
    in your application then for each of (1,2) specific configuration file must be presented.

      dbccodeconf.h                                (5) app / inc

    This is application specific configuration file. This file may include "CanFrame" definition,
    sigfloat_t typedef and binutil macros which enables rx and tx structures allocation inside 
    ecudb-binutil.c. This file must be single for application (see template dbccodeconf.h), source
    code (4,6) includes this configuration.

      ecudb-binutil.c / ecudb-binutil.h            (6) RO / app

    Basically this is application specific file. This driver has one function which intakes CAN
    message data and calls dedicated unpacking function. Function selection based on binary search. 
    
      canmonitorutil.h                             (7) lib

    This is lib level source code. This file is basic for all automatic monitoring functions. 
    This configuration file location have to be added to project include path.
    
    -----------------------------------------------------------------------------------------------

    *inc - file location have to be added to project include path.

  ## "-nodeutils" option

  If your matrix has strict routing setup, where each CAN device (node) has defined collection 
  of TX frames as well as defined collection of RX frames the "-nodeutils" option may be used.
  In this mode all the nodes defined in CAN matrix will be handled as specific ECU and 
  for each of these specific ECUs dedicated "###-binutil.c/h" pair of source code will be generated.
  
  See help output using details.

  ## "-rxcb" option

  If you want to generate function callback stubs for each signal, using `-rxcb` with codedbc
  will generate NULL defined stubs. Assign any of the generated callbacks to have them be executed
  automatically when the associated signal is unpacked. See code snippet below for example use.

  ```c
  #include "conf/canmessages-config.h"
  #include "lib/canmessages.h"
  #include "butl/nodea_canmessages-binutil.h"
  #include "butl/nodeb_canmessages-binutil.h"
  #include "common_tuner.h"

  // Signal NodeAtoTuner_SeekRequest; Callback for function NodeAtoTuner_SeekRequest_cb
  void SiLabs_Tuner_SeekRequest(uint8_t direction)
  {
      uint8_t seek_dir = (direction == CANMessages_Signal_NodeA_SeekRequest_Up) ? 1 : 0;
      RETURN_TYPE ret = SiLabs_Tuner_fm_seek_start(tuner_handle, seek_dir);
      if (ret != SUCCESS)
      {
          LOG_MACRO("C-CODERDBC", "%s:%d > %s(): Tuner failed to seek (direction %d); ret=%d", __FILE__, __LINE__, __func__, direction, ret);
      }
  }

  int main(int argc, char* argv[])
  {
      // Assign CAN database callbacks
      DisplayToTuner_SeekRequest_cb = Si479xx_SeekRequest;

      while (true)
      {
          // Receive a CAN frame from the CAN Bus
          uint8_t canFrame_dataLength, canFrame_data[8];
          uint32_t canFrame_id;
          CAN_HAL_RX_CANFrame(canFrame_data, &canFrame_id, &canFrame_dataLength);

          // Parse the received CAN frame with the CAN database generated code; callbacks triggered automatically
          canmessages_rx_t msg;
          canmessages_Receive(&msg, canFrame_data, canFrame_id, canFrame_dataLength);
      }
  }
  ```

  ## "-canstruct <path/to/header.h>" option

  Use this option if you have a header which defines the `__CoderDbcCanFrame_t__` struct. This will insert your
  header into the generated files so you don't have to modify the generated code to define/redefine generated code.

  ## "-usestruct" option

  Use this option to enable the macro which allows for use of a CAN frame struct instead of passing the individual variables.

  ## "-defstruct <typedef struct {/\*definition\*/} __CoderDbcCanFrame_t__;>" option

  Use this option to define the CAN struct during code generation instead of modifying generated code or providing a path to 
  a header which defines it instead.  
  Can be used with option `-canstruct` to provide an existing struct definition and assigning __CoderDbcCanFrame_t__ to it:  
  `-canstruct "driver/twai.h" -defstruct "typedef twai_message_t __CoderDbcCanFrame_t__;"`

  ## "-txcb" option

  If you want to generate a function callback stub for sending a CAN frame after packing a signal,
  using `-txcb` with codedbc will generate a NULL defined stub. Assign the generated callback to have
  it execute automatically after packing a signal. See code snippet below for example use.

  ```c++
  #include "conf/canmessages-config.h"
  #include "lib/canmessages.h"
  #include "butl/nodea_canmessages-binutil.h"
  #include "butl/nodeb_canmessages-binutil.h"
  #include "common_tuner.h"
  #include "driver/gpio.h"
  #include "driver/twai.h"
  #include "freertos/FreeRTOS.h"
  #include "freertos/queue.h"
  #include "freertos/semphr.h"
  #include "freertos/task.h"
  #include "hal/gpio_types.h"

  #if CANMESSAGES_USE_CANSTRUCT
  extern "C" void SendMessage(__CoderDbcCanFrame_t__ frame)
  {
  #else
  extern "C" void SendMessage(uint32_t _id, uint8_t* _d, uint8_t* _len, uint8_t* _ide)
  {
    twai_message_t frame = {
      .extd = *ide,
      .identifier = _id,
      .data_length_code = *_len,
      .data = _d
    };
  #endif
      xSemaphoreTake(g_canSemaphore, portMAX_DELAY);
      twai_transmit(&(twai_message_t)frame, pdMS_TO_TICKS(DEFINED_DELAY));
      xSemaphoreGive(g_canSemaphore);
  }


  int main(int argc, char* argv[])
  {
      Init_Drivers();

      // Assign CAN database TX callback
      CANMessages_TX_cb = SendMessage;

      while (true)
      {
          /* Stuff being done... */

          if (tune_to_channel)
          {
            // Display needs to tell Tuner to tune to station 100.5
            NodeA_TuneToFrequency_t msg = { .Frequency = 100500 };
            #if CANMESSAGES_USE_CANSTRUCT
            __CoderDbcCanFrame_t__ frame;
            Pack_NodeA_TuneToFrequency_CANMessages(&msg, &frame);
            #else
            uint8_t data[8], len=8, ide;
            Pack_NodeA_TuneToFrequency_CANMessages(&msg, NodeA_TuneToFrequency_CANID, data, &len, &ide);
            #endif

            /* Alternatively, using the helper macro to send a CAN frame */
            CANdySend_CANMessages(NodeA_TuneToFrequency, { .Frequency = 100500 });
          }

          /* Handling a received signal */

          uint8_t canFrame_dataLength, canFrame_data[8];
          uint32_t canFrame_id;
          CAN_HAL_RX_CANFrame(canFrame_data, &canFrame_id, &canFrame_dataLength);

          // Parse the received CAN frame with the CAN database generated code; callbacks triggered automatically
          canmessages_rx_t msg;
          canmessages_Receive(&msg, canFrame_data, canFrame_id, canFrame_dataLength);
      }
  }
  ```

