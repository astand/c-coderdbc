#pragma once

#include "../types/c-expr.h"

class CSigPrinter
{
public:
	CSigPrinter();
	~CSigPrinter();

	void AddMessage(const MessageDescriptor_t& message);
	void AddMessage(const std::vector<MessageDescriptor_t*> message);

public: 
	std::vector<CiExpr_t*> sigs_expr;

private:
	std::string GetSignalType(const SignalDescriptor_t& signal);

};
