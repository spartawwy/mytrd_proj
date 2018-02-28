#ifndef AGENT_UTILITY_USE_SDF3KSDFDS_H_
#define AGENT_UTILITY_USE_SDF3KSDFDS_H_

#include <string>
#include <vector>

namespace UtilityUse
{
std::string&   replace_all(std::string&   str, const   std::string&   old_value, const   std::string&   new_value);
std::string&   replace_all_distinct(std::string&   str, const   std::string&   old_value, const  std::string&   new_value);
std::vector<std::string> split(const std::string& source, const std::string& seperator = " ", 
							   const std::string& quotation = "\"");
}
#endif // AGENT_UTILITY_USE_SDF3KSDFDS_H_