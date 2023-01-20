// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#include "irc_message.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<std::string> cowircd::irc_message::split(const std::string& str)
{
    std::vector<std::string> vec;
    std::string::size_type pos = 0;
    for (;;)
    {
        std::string::size_type found;
        if (!vec.empty() && str[pos] == ':')
        {
            pos++;
            found = std::string::npos;
        }
        else
        {
            found = str.find(' ', pos);
        }

        if (found != pos)
        {
            vec.push_back(str.substr(pos, found - pos));
        }

        if (found != std::string::npos)
        {
            pos = found + 1;
            if (pos == str.size())
            {
                pos = std::string::npos;
            }
        }
        else
        {
            break;
        }
    }

    return vec;
}

bool cowircd::irc_message::validate_command(std::string& command)
{
    if (std::isalpha(command[0]))
    {
        for (std::string::iterator it = command.begin(); it != command.end(); ++it)
        {
            if (!std::isalpha(*it))
            {
                return false;
            }
            *it = std::toupper(*it);
        }
    }
    else if (std::isdigit(command[0]))
    {
        if (command.size() != 3)
        {
            return false;
        }
        for (std::string::iterator it = command.begin(); it != command.end(); ++it)
        {
            if (!std::isdigit(*it))
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool cowircd::irc_message::parse(const std::string& str, irc_message& out_msg)
{
    std::vector<std::string> vec = split(str);
    std::vector<std::string>::iterator it = vec.begin();

    if (it == vec.end() || it->empty())
    {
        return false;
    }

    bool has_prefix = false;
    std::string prefix;
    if (it->operator[](0) == ':')
    {
        has_prefix = true;
        prefix = it->substr(1);
        ++it;

        if (it == vec.end() || it->empty())
        {
            return false;
        }
    }

    std::string command = *it++;
    if (!validate_command(command))
    {
        return false;
    }

    out_msg = irc_message(command);
    if (has_prefix)
    {
        out_msg.set_prefix(prefix);
    }
    if (it != vec.end())
    {
        out_msg.reserve_param(vec.end() - it);
        for (; it != vec.end(); ++it)
        {
            out_msg.add_param(*it);
        }
    }

    return true;
}

cowircd::irc_message::irc_message()
{
}

cowircd::irc_message::irc_message(const std::string& command)
    : command(command), has_prefix_value(false), prefix(), params(), last_param_is_trailing(false)
{
}

std::string& cowircd::irc_message::operator[](int i)
{
    const irc_message* const_this = this;
    return const_cast<std::string&>(const_this->operator[](i));
}

const std::string& cowircd::irc_message::operator[](int i) const
{
    if (i == -1)
    {
        return this->prefix;
    }
    else if (i == 0)
    {
        return this->command;
    }
    else
    {
        return this->params[i - 1];
    }
}

const std::string& cowircd::irc_message::get_command() const
{
    return this->command;
}

bool cowircd::irc_message::has_prefix() const
{
    return this->has_prefix_value;
}

const std::string& cowircd::irc_message::get_prefix() const
{
    return this->prefix;
}

void cowircd::irc_message::set_prefix(const std::string& prefix)
{
    if (prefix.find(' ') != std::string::npos)
    {
        throw std::runtime_error("접두어에는 공백이 있을 수 없습니다.");
    }

    this->has_prefix_value = true;
    this->prefix = prefix;
}

void cowircd::irc_message::reset_prefix()
{
    this->has_prefix_value = false;
}

std::size_t cowircd::irc_message::size_param() const
{
    return this->params.size();
}

void cowircd::irc_message::reserve_param(std::size_t n)
{
    this->params.reserve(n);
}

void cowircd::irc_message::add_param(const std::string& param)
{
    if (this->last_param_is_trailing)
    {
        throw std::runtime_error("이미 최후의 매개변수를 받았습니다.");
    }

    this->params.push_back(param);
    this->last_param_is_trailing = param.find(' ') != std::string::npos;
}

void cowircd::irc_message::remove_all_param()
{
    this->params.clear();
    this->last_param_is_trailing = false;
}

std::string cowircd::irc_message::to_string() const
{
    std::ostringstream oss;
    if (this->has_prefix_value)
    {
        oss << ":" << this->prefix << " ";
    }
    oss << this->command;
    for (std::vector<std::string>::const_iterator it = this->params.begin(); it != this->params.end(); ++it)
    {
        oss << ((this->last_param_is_trailing && it + 1 == this->params.end()) ? " :" : " ") << *it;
    }
    return oss.str();
}

std::string cowircd::irc_message::to_pretty_string() const
{
    std::ostringstream oss;
    oss << "{" << std::endl;
    oss << "\tCOMMAND=\"" << this->get_command() << "\"" << std::endl;
    if (this->has_prefix())
    {
        oss << "\tPREFIX=\"" << this->get_prefix() << "\"" << std::endl;
    }
    else
    {
        oss << "\tPREFIX=(null)" << std::endl;
    }
    oss << "\tPARAMS = [" << this->size_param() << "] {" << std::endl;
    for (std::size_t i = 1; i <= this->size_param(); i++)
    {
        oss << "\t\t[" << i << "] = \"" << this->operator[](i) << "\"" << std::endl;
    }
    oss << "\t}" << std::endl;
    oss << "}" << std::endl;
    return oss.str();
}
