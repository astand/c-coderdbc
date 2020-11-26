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

	int32_t BuildCConvertExprs(CiExpr_t* msg);

	std::string PrintSignalExpr(SignalDescriptor_t* sig, std::vector<std::string>& to_bytes);

	void AppendToByteLine(std::string& expr, std::string str);

};
