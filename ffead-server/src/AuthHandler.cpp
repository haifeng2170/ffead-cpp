/*
	Copyright 2009-2012, Sumeet Chhetri

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
/*
 * AuthHandler.cpp
 *
 *  Created on: Jun 17, 2012
 *      Author: Sumeet
 */

#include "AuthHandler.h"

AuthHandler::AuthHandler() {
	// TODO Auto-generated constructor stub

}

AuthHandler::~AuthHandler() {
	// TODO Auto-generated destructor stub
}

string AuthHandler::getFileExtension(const string& file)
{
	string str = file;
	string ext = "";
	for(unsigned int i=0; i<str.length(); i++)
	{
		if(str[i] == '.')
		{
			for(unsigned int j = i; j<str.length(); j++)
			{
				ext += str[j];
			}
			return ext;
		}
	}
	return ext;
}

bool AuthHandler::handle(ConfigurationData configData, HttpRequest* req, HttpResponse& res, string ext)
{
	map<string, string> autMap = configData.autMap;
	map<string, string> autpattMap = configData.autpattMap;
	map<string, vector<string> > filterMap = configData.filterMap;

	Logger logger = LoggerFactory::getLogger("AuthHandler");
	bool isContrl = false;
	string claz;
	if(autpattMap[req->getCntxt_name()+"*.*"]!="" || autMap[req->getCntxt_name()+ext]!="")
	{
		if(autpattMap[req->getCntxt_name()+"*.*"]!="")
		{
			claz = autpattMap[req->getCntxt_name()+"*.*"];
		}
		else
		{
			claz = autMap[req->getCntxt_name()+ext];
		}

		logger << ("OAUTH/HTTP Authorization requested " +  claz) << endl;
		map<string,string>::iterator it;
		map<string,string> tempmap = req->getAuthinfo();
		for(it=tempmap.begin();it!=tempmap.end();it++)
		{
			logger << it->first << " = " << it->second << endl;
		}
		map<string,string> tempmap1 = req->getAllParams();
		for(it=tempmap1.begin();it!=tempmap1.end();it++)
		{
			logger << it->first << " = " << it->second << endl;
		}
		if(claz.find("file:")!=string::npos)
		{
			claz = req->getCntxt_root()+"/"+claz.substr(claz.find(":")+1);
			logger << ("Auth handled by file " + claz) << endl;
			FileAuthController authc(claz,":");
			if(authc.isInitialized())
			{
				if(authc.authenticate(req->getAuthinfo()["Username"],req->getAuthinfo()["Password"]))
				{
					logger << "Valid user" << endl;
				}
				else
				{
					logger << "Invalid user" << endl;
					res.setHTTPResponseStatus(HTTPResponseStatus::AccessDenied);
					isContrl = true;
					logger << "Verified request token signature is invalid" << endl;
				}
			}
			else
			{
				logger << "Invalid user repo defined" << endl;
			}
		}
		else if(claz.find("class:")!=string::npos)
		{
			claz = claz.substr(claz.find(":")+1);
			void *_temp = configData.ffeadContext->getBean("authhandler_"+req->getCntxt_name()+claz, req->getCntxt_name());
			AuthController *authc = static_cast<AuthController*>(_temp);
			if(authc!=NULL)
			{
				isContrl = authc->handle(req,&res);
				if(isContrl && res.getStatusCode()!="")
					isContrl = true;
				logger << "Authhandler called" << endl;
			}
			else
			{
				logger << "Invalid Auth handler" << endl;
				res.setHTTPResponseStatus(HTTPResponseStatus::InternalServerError);
				isContrl = true;
			}
		}
	}
	return isContrl;
}
