#include <iostream>
#include "SignatureManager.h"

void SignatureManager::addSignature(const Signature& sig)
{
	addSignature(std::make_shared<Signature>(sig));
}

void SignatureManager::addSignature(const std::string expression)
{
	SignaturePtr sig = SignaturePtr(new Signature(expression));

	addSignature(sig);
}

void SignatureManager::addSignature(SignaturePtr sig)
{
	signatures_.push_back(sig);
}

void SignatureManager::evaluate(const unsigned char *payload, bool *result)
{
//	std::cout << "Processing query(" << query.c_str() << ")" <<std::endl;

        std::find_if(signatures_.begin(),
                signatures_.end(),  [&](SignaturePtr& sig)
        {
//		std::cout << "Evaluating rule(" << r->getExpression() << ")" << std::endl;
		if(sig->evaluate(payload))
		{
			++total_matched_signatures_;
			current_signature_ = sig;
			(*result) = true;
//			std::cout << "Matchs(" << query.c_str() <<")" << *result <<std::endl;
			// return from the find_if	
			return true;
		}
        });

	return;
}

SignaturePtr SignatureManager::getMachtedSignature() 
{ 
	return current_signature_;
}