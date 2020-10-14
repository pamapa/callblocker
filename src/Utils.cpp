/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2020 Patrick Ammann <pammann@gmx.net>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "Utils.h" // API

#include <arpa/inet.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <netdb.h>
#include <regex>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <json-c/json.h>
#include <pjsua-lib/pjsua.h>
#if defined(HAVE_LIBPHONENUMBER)
#include <phonenumbers/phonenumberutil.h>
#endif

#include "Logger.h"

#define PATH_SEPERATOR '/'

std::string Utils::pathJoin(const std::string& rPath, const std::string& rFilename)
{
    if (rPath.back() == PATH_SEPERATOR || rFilename.front() == PATH_SEPERATOR) {
        return rPath + rFilename;
    }
    return rPath + PATH_SEPERATOR + rFilename;
}

std::string Utils::pathBasename(const std::string& rPath)
{
    std::string::size_type pos = rPath.rfind(PATH_SEPERATOR);
    if (pos == std::string::npos) {
        return rPath;
    }
    return rPath.substr(pos + 1);
}

std::string Utils::pathDirname(const std::string& rPath)
{
    std::string::size_type pos = rPath.rfind(PATH_SEPERATOR);
    if (pos == std::string::npos) {
        return "";
    }
    return rPath.substr(0, pos);
}

std::string Utils::pathAbsname(const std::string& rPath)
{
    char resolved_path[PATH_MAX];
    if (realpath(rPath.c_str(), resolved_path) != 0) {
        return resolved_path;
    }
    return "";
}

// file or directory
bool Utils::pathExists(const std::string& rPath)
{
    struct stat st;
    if (stat(rPath.c_str(), &st) == 0) {
        return true;
    }
    return false;
}

bool Utils::fileCopy(const std::string& rFrom, const std::string& rTo)
{
    //printf("copy from %s to %s", rFrom.c_str(), rTo.c_str());
    std::ifstream ifs(rFrom.c_str(), std::ios::binary);
    if (ifs.fail()) {
        return false;
    }
    std::ofstream ofs(rTo.c_str(), std::ios::binary);
    if (ofs.fail()) {
        return false;
    }
    ofs << ifs.rdbuf();
    return true;
}

bool Utils::loadJson(const std::string& filename, struct json_object** pRoot)
{
    std::ifstream in(filename);
    if (in.fail()) {
        Logger::warn("Utils::loadJson(%s): loading failed", filename.c_str());
        return false;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string str = buffer.str();
    in.close();

    return Utils::parseJson(str, pRoot);
}

bool Utils::parseJson(const std::string& str, struct json_object** pRoot)
{
    *pRoot = json_tokener_parse(str.c_str());
    if (*pRoot == nullptr) {
        Logger::warn("Utils::parseJson(%s): could not parse", str.c_str());
        return false;
    }
    return true;
}

bool Utils::getObject(struct json_object* objbase, const char* objname, bool logNotFoundError, const std::string& rLocation,
    std::string* pRes, const std::string& rDefault)
{
    struct json_object* n;
    *pRes = rDefault;
    if (!json_object_object_get_ex(objbase, objname, &n)) {
        if (logNotFoundError)
            Logger::warn("%s not found in %s", objname, rLocation.c_str());
        return false;
    }
    if (json_object_get_type(n) != json_type_string) {
        Logger::warn("string type expected for %s in %s", objname, rLocation.c_str());
        return false;
    }
    *pRes = json_object_get_string(n);
    return true;
}

bool Utils::getObject(struct json_object* objbase, const char* objname, bool logNotFoundError, const std::string& rLocation,
    int* pRes, const int rDefault)
{
    struct json_object* n;
    *pRes = rDefault;
    if (!json_object_object_get_ex(objbase, objname, &n)) {
        if (logNotFoundError)
            Logger::warn("%s not found in %s", objname, rLocation.c_str());
        return false;
    }
    if (json_object_get_type(n) != json_type_int) {
        Logger::warn("integer type expected for %s in %s", objname, rLocation.c_str());
        return false;
    }
    *pRes = json_object_get_int(n);
    return true;
}

bool Utils::getObject(struct json_object* objbase, const char* objname, bool logNotFoundError, const std::string& rLocation,
    bool* pRes, const bool rDefault)
{
    struct json_object* n;
    *pRes = rDefault;
    if (!json_object_object_get_ex(objbase, objname, &n)) {
        if (logNotFoundError)
            Logger::warn("%s not found in %s", objname, rLocation.c_str());
        return false;
    }
    if (json_object_get_type(n) != json_type_boolean) {
        Logger::warn("boolean type expected for %s in %s", objname, rLocation.c_str());
        return false;
    }
    *pRes = (bool)json_object_get_boolean(n);
    return true;
}

bool Utils::getObject(struct json_object* objbase, const char* objname, bool logNotFoundError, const std::string& rLocation,
    std::chrono::system_clock::time_point* pRes, const std::chrono::system_clock::time_point& rDefault)
{
    struct json_object* n;
    *pRes = rDefault;
    if (!json_object_object_get_ex(objbase, objname, &n)) {
        if (logNotFoundError)
            Logger::warn("%s not found in %s", objname, rLocation.c_str());
        return false;
    }
    if (json_object_get_type(n) != json_type_string) {
        Logger::warn("string type expected for %s in %s", objname, rLocation.c_str());
        return false;
    }
    std::string str = json_object_get_string(n);
    if (!Utils::parseTime(str, pRes)) {
        Logger::warn("invalid timestamp string found '%s' for %s in %s", str.c_str(), objname, rLocation.c_str());
        return false;
    }
    return true;
}

bool Utils::executeCommand(const std::string& rCmd, std::string* pRes)
{
    Logger::debug("executing(%s)...", rCmd.c_str());

    std::string cmd;
    // Python3 scripts need this for sys.stdout.write with non ascii characters
    cmd = "PYTHONIOENCODING=\"utf-8\"";
    cmd += " " + rCmd;

    FILE* fp = popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        Logger::warn("popen failed (%s)", strerror(errno));
        return false;
    }

    std::string res = "";
    char buf[128];
    while (fgets(buf, sizeof(buf), fp) != nullptr) {
        res += buf;
    }
    Utils::trim(&res);

    int status = pclose(fp);
    if (status != 0) {
        Logger::warn("%s failed (%s)", cmd.c_str(), res.c_str());
        return false;
    }

    Logger::debug("result: '%s'", res.c_str());
    *pRes = res;
    return true;
}

std::string Utils::getPjStatusAsString(pj_status_t status)
{
    static char buf[100];
    pj_str_t pjstr = pj_strerror(status, buf, sizeof(buf));
    std::string ret = pj_strbuf(&pjstr);
    return ret;
}

bool Utils::startsWith(const std::string& rStr, const char* pPrefix)
{
    return rStr.find(pPrefix) == 0 ? true : false;
}

static void leftTrim(std::string* pStr)
{
    std::string::size_type pos = pStr->find_last_not_of(" \t\r\n");
    if (pos != std::string::npos) {
        pStr->erase(pos + 1);
    }
}

static void rightTrim(std::string* pStr)
{
    std::string::size_type pos = pStr->find_first_not_of(" \t\r\n");
    if (pos != std::string::npos) {
        pStr->erase(0, pos);
    }
}

void Utils::trim(std::string* pStr)
{
    leftTrim(pStr);
    rightTrim(pStr);
}

std::string Utils::escapeSqString(const std::string& rStr)
{
    std::size_t n = rStr.length();
    std::string escaped;
    escaped.reserve(n * 2); // pessimistic preallocation
    for (size_t i = 0; i < n; ++i) {
        if (rStr[i] == '\\' || rStr[i] == '\'') {
            escaped += '\\';
        }
        escaped += rStr[i];
    }
    return escaped;
}

void Utils::replaceAll(std::string* pStr, const std::string& rFrom, const std::string& rTo)
{
    size_t startPos = 0;
    while ((startPos = pStr->find(rFrom, startPos)) != std::string::npos) {
        pStr->replace(startPos, rFrom.length(), rTo);
        startPos += rTo.length(); // Handles case where 'to' is a substring of 'from'
    }
}

bool Utils::matchWithWildcards(const std::string& rStr, const std::string& rWildcardPattern, bool caseSensitive)
{
    // escape all regex special chars
    std::string pattern = rWildcardPattern;
    Utils::replaceAll(&pattern, "\\", "\\\\");
    Utils::replaceAll(&pattern, "^", "\\^");
    Utils::replaceAll(&pattern, ".", "\\.");
    Utils::replaceAll(&pattern, "$", "\\$");
    Utils::replaceAll(&pattern, "|", "\\|");
    Utils::replaceAll(&pattern, "(", "\\(");
    Utils::replaceAll(&pattern, ")", "\\)");
    Utils::replaceAll(&pattern, "{", "\\{");
    Utils::replaceAll(&pattern, "{", "\\}");
    Utils::replaceAll(&pattern, "[", "\\[");
    Utils::replaceAll(&pattern, "]", "\\]");
    Utils::replaceAll(&pattern, "+", "\\+");
    Utils::replaceAll(&pattern, "/", "\\/");
    // convert chars '*?' to their regex equivalents
    Utils::replaceAll(&pattern, "?", ".");
    Utils::replaceAll(&pattern, "*", ".*");

    bool ret = false;
    try {
        std::regex re(pattern, caseSensitive ? std::regex_constants::ECMAScript : std::regex_constants::ECMAScript | std::regex_constants::icase);
        ret = regex_match(rStr, re);
    } catch (std::regex_error& ex) {
        // syntax error in the regular expression
    }
    return ret;
}

// returns phone number in E.164 format
void Utils::makeNumberE164(const struct SettingBase* pSettings, std::string* pNumber, bool* pValid)
{
    if (Utils::startsWith(*pNumber, "**")) {
        // it is an intern number
        *pValid = true;
        return;
    }

#if defined(HAVE_LIBPHONENUMBER)
    // country code as interger
    std::string tmp = pSettings->countryCode;
    tmp.erase(0, 1); // remove '+'
    int country_code = std::stoi(tmp);

    // get region code from country code
    i18n::phonenumbers::PhoneNumberUtil* pPhoneUtil = i18n::phonenumbers::PhoneNumberUtil::GetInstance();
    std::string region_code;
    pPhoneUtil->GetRegionCodeForCountryCode(country_code, &region_code);

    i18n::phonenumbers::PhoneNumber n;
    i18n::phonenumbers::PhoneNumberUtil::ErrorType err = pPhoneUtil->Parse(*pNumber, region_code, &n);
    if (err != i18n::phonenumbers::PhoneNumberUtil::ErrorType::NO_PARSING_ERROR) {
        *pValid = false;
        return;
    }
    *pValid = pPhoneUtil->IsValidNumber(n);
    if (*pValid) {
        pPhoneUtil->Format(n, i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat::E164, pNumber);
    }
#else
    std::string number;
    // very simple: does not work for all countries (libphonenumber will do better job)
    // see http://www.nationsonline.org/oneworld/international-calling-codes.htm
    if (Utils::startsWith(*pNumber, "+"))
        number = *pNumber;
    else if (Utils::startsWith(*pNumber, "00"))
        number = "+" + pNumber->substr(2);
    else if (Utils::startsWith(*pNumber, "0"))
        number = pSettings->countryCode + pNumber->substr(1);
    else
        number = "+" + *pNumber;

    // minimal validity check
    bool valid = true;
    if (number.length() < (1 + 8) || (1 + 15) < number.length()) { // 1+: "+" prefix
        // E.164: too short or too long
        valid = false;
    }

    // unassigned (https://en.wikipedia.org/wiki/List_of_country_calling_codes)
    static const char* unassignedCountryCodes[] {
        // Zone 2
        "+210", "+214", "+215", "+217", "+219",
        "+259",
        "+28",
        "+292", "+293", "+294", "+296",
        // Zones 3-4
        "+384",
        "+422", "+424", "+425", "+426", "+427", "+428", "+429",
        // Zone 6
        "+693", "+694", "+695", "+696", "+697", "+698", "+699",
        // Zone 8
        "+801", "+802", "+803", "+804", "+805", "+806", "+807", "+809",
        "+851", "+854", "+857", "+858", "+859",
        "+871", "+872", "+873", "+874", "+884", "+885", "+887", "+889", "+89x",
        // Zone 9
        "+969", "+978", "+990", "+997", "+999"
    };
    if (valid) {
        for (const auto& unassignedCountryCode : unassignedCountryCodes) {
            if (Utils::startsWith(number, unassignedCountryCode)) {
                valid = false;
                break;
            }
        }
    }

    if (valid)
        *pNumber = number;
    *pValid = valid;
#endif
}

void Utils::parseCallerID(std::string& rData, std::vector<std::pair<std::string, std::string>>* pResult)
{
    // DATE=0306
    // TIME=1517
    // NMBR=0123456789
    // NAME=aasdasdd

    // split by newline
    std::stringstream ss(rData);
    std::string to;
    while (std::getline(ss, to, '\n')) {
        // key=val
        std::string::size_type pos = to.find('=');
        if (pos == std::string::npos)
            continue;
        std::string key = to.substr(0, pos);
        std::string val = "";
        if (pos + 1 < to.length()) {
            val = to.substr(pos + 1, to.length() - pos);
        }
        Utils::trim(&key);
        Utils::trim(&val);
        std::pair<std::string, std::string> p(key, val);
        pResult->push_back(p);
    }
}

std::string Utils::formatTime(const std::chrono::system_clock::time_point& rTp)
{
    // "2015-05-15 12:00:00 +0000"

    tm tm_tmp;
    time_t time_t_tmp = std::chrono::system_clock::to_time_t(rTp);
    gmtime_r(&time_t_tmp, &tm_tmp);

    char timeFormat[32];
    strftime(timeFormat, sizeof(timeFormat), "%Y-%m-%d %H:%M:%S +0000", &tm_tmp);
    return std::string(timeFormat);
}

bool Utils::parseTime(const std::string& rStr, std::chrono::system_clock::time_point* pRes)
{
    // "2015-05-15 12:00:00 +0000"

    std::tm tm_tmp;
    if (strptime(rStr.c_str(), "%Y-%m-%d %H:%M:%S %z", &tm_tmp) == nullptr) {
        return false;
    }

    *pRes = std::chrono::system_clock::from_time_t(timegm(&tm_tmp));
    return true;
}

bool Utils::resolveHostname(const std::string& rHostname, int ai_family, std::string* pRes)
{
    struct addrinfo hints, *servinfo;

    (void)memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(rHostname.c_str(), "http", &hints, &servinfo) != 0) {
        Logger::warn("getaddrinfo: %s\n", strerror(errno));
        return false;
    }

    if (servinfo == nullptr) {
        Logger::warn("no address found for: %s\n", rHostname.c_str());
        return false;
    }

    std::string res;
    struct sockaddr_in* h = (struct sockaddr_in*)servinfo->ai_addr;
    res = inet_ntoa(h->sin_addr);

    *pRes = res;
    return true;
}
