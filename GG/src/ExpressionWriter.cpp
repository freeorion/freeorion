// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */

#include <GG/ExpressionWriter.h>

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/implementation/token.hpp>

#include <boost/lexical_cast.hpp>

#include <deque>
#include <map>


using namespace GG;

namespace {

    const std::map<adobe::name_t, const char*>& BinaryOps()
    {
        static std::map<adobe::name_t, const char*> s_ops;
        if (s_ops.empty()) {
            s_ops[adobe::or_k] = "|| ";
            s_ops[adobe::and_k] = "&& ";
            s_ops[adobe::equal_k] = "== ";
            s_ops[adobe::not_equal_k] = "!= ";
            s_ops[adobe::less_k] = "< ";
            s_ops[adobe::less_equal_k] = "<= ";
            s_ops[adobe::greater_k] = "> ";
            s_ops[adobe::greater_equal_k] = ">= ";
            s_ops[adobe::add_k] = "+ ";
            s_ops[adobe::subtract_k] = "- ";
            s_ops[adobe::multiply_k] = "* ";
            s_ops[adobe::divide_k] = "/ ";
            s_ops[adobe::modulus_k] = "% ";
        }
        return s_ops;
    }

    bool BinaryOp(adobe::name_t name)
    { return BinaryOps().find(name) != BinaryOps().end(); }

    const char* BinaryOpString(adobe::name_t name)
    { return BinaryOps().find(name)->second; }

    const std::map<adobe::name_t, const char*>& UnaryOps()
    {
        static std::map<adobe::name_t, const char*> s_ops;
        if (s_ops.empty()) {
            s_ops[adobe::unary_negate_k] = "-";
            s_ops[adobe::not_k] = "!";
        }
        return s_ops;
    }

    bool UnaryOp(adobe::name_t name)
    { return UnaryOps().find(name) != UnaryOps().end(); }

    const char* UnaryOpString(adobe::name_t name)
    { return UnaryOps().find(name)->second; }

    void WriteExpressionImpl(adobe::array_t::const_reverse_iterator& it,
                             adobe::array_t::const_reverse_iterator end_it,
                             std::string& output);

    void WriteArrayBody(adobe::array_t::const_reverse_iterator& it,
                        adobe::array_t::const_reverse_iterator end_it,
                        std::string& output)
    {
        ++it;
        const std::size_t size = it->cast<std::size_t>();
        ++it;
        std::deque<std::string> elements;
        for (std::size_t i = 0; i < size; ++i) {
            elements.push_front(std::string());
            WriteExpressionImpl(it, end_it, elements.front());
        }
        output += elements[0];
        for (std::size_t i = 1; i < size; ++i) {
            if (output[output.size() - 1] == ' ')
                output.resize(output.size() - 1);
            output += ", ";
            output += elements[i];
        }
    }

    void WriteDictionaryBody(adobe::array_t::const_reverse_iterator& it,
                             adobe::array_t::const_reverse_iterator end_it,
                             std::string& output)
    {
        ++it;
        const std::size_t size = it->cast<std::size_t>();
        ++it;
        std::deque<std::string> elements;
        for (std::size_t i = 0; i < size; ++i) {
            elements.push_front(std::string());
            WriteExpressionImpl(it, end_it, elements.front());
            elements.push_front(it->cast<adobe::name_t>().c_str());
            ++it;
        }
        output += elements[0];
        output += " : ";
        output += elements[1];
        for (std::size_t i = 1; i < size; ++i) {
            if (output[output.size() - 1] == ' ')
                output.resize(output.size() - 1);
            output += ", ";
            output += elements[i * 2 + 0];
            output += " : ";
            output += elements[i * 2 + 1];
        }
    }

    void WritePrimaryExpression(adobe::array_t::const_reverse_iterator& it,
                                adobe::array_t::const_reverse_iterator end_it,
                                std::string& output)
    {
        adobe::name_t op;
        if (it->cast(op)) {
            if (op == adobe::function_k) {
                ++it;
                output += it->cast<adobe::name_t>().c_str();
                ++it;
                output += "( ";
                if (it->type_info() == adobe::type_info<adobe::array_t>()) {
                    ++it;
                } else {
                    adobe::name_t arg_type;
                    it->cast(arg_type);
                    if (arg_type == adobe::array_k)
                        WriteArrayBody(it, end_it, output);
                    else
                        WriteDictionaryBody(it, end_it, output);
                }
                if (output[output.size() - 1] != ' ')
                    output += ' ';
                output += ") ";
            } else if (op == adobe::variable_k) {
                ++it;
                output += it->cast<adobe::name_t>().c_str();
                output += ' ';
                ++it;
            } else if (op == adobe::dictionary_k) {
                output += "{ ";
                WriteDictionaryBody(it, end_it, output);
                output += "} ";
            } else if (op == adobe::array_k) {
                output += "[ ";
                WriteArrayBody(it, end_it, output);
                output += "] ";
            } else if (op == adobe::parenthesized_expression_k) {
                ++it;
                output += '(';
                WriteExpressionImpl(it, end_it, output);
                output += ") ";
            } else {
                if (op == adobe::name_k) {
                    output += '@';
                    ++it;
                }
                output += it->cast<adobe::name_t>().c_str();
                ++it;
                output += ' ';
            }
        } else if (it->type_info() == adobe::type_info<adobe::dictionary_t>()) {
            output += "{} ";
            ++it;
        } else if (it->type_info() == adobe::type_info<adobe::array_t>()) {
            output += "[] ";
            ++it;
        } else if (*it == adobe::any_regular_t()) {
            output += "empty ";
            ++it;
        } else if (it->type_info() == adobe::type_info<adobe::string_t>()) {
            std::string text = it->cast<adobe::string_t>();
            char quote_char = '\'';
            if (text.find('\'') != std::string::npos)
                quote_char = '"';
            output += quote_char;
            output += text;
            output += quote_char;
            output += ' ';
            ++it;
        } else if (it->type_info() == adobe::type_info<bool>()) {
            output += it->cast<bool>() ? "true " : "false ";
            ++it;
        } else if (it->type_info() == adobe::type_info<double>()) {
            output += boost::lexical_cast<std::string>(it->cast<double>());
            output += ' ';
            ++it;
        }
    }

    void WriteExpressionImpl(adobe::array_t::const_reverse_iterator& it,
                             adobe::array_t::const_reverse_iterator end_it,
                             std::string& output)
    {
        if (it == end_it)
            return;

        adobe::name_t op;
        if (it->cast(op)) {
            if (op == adobe::ifelse_k) {
                ++it;
                std::string else_expr;
                {
                    const adobe::array_t& subexpr = (it++)->cast<adobe::array_t>();
                    adobe::array_t::const_reverse_iterator subexpr_it = subexpr.rbegin();
                    adobe::array_t::const_reverse_iterator subexpr_end_it = subexpr.rend();
                    WriteExpressionImpl(subexpr_it, subexpr_end_it, else_expr);
                }
                std::string then_expr;
                {
                    const adobe::array_t& subexpr = (it++)->cast<adobe::array_t>();
                    adobe::array_t::const_reverse_iterator subexpr_it = subexpr.rbegin();
                    adobe::array_t::const_reverse_iterator subexpr_end_it = subexpr.rend();
                    WriteExpressionImpl(subexpr_it, subexpr_end_it, then_expr);
                }
                WriteExpressionImpl(it, end_it, output);
                output += "? ";
                output += then_expr;
                output += ": ";
                output += else_expr;
            } else if (BinaryOp(op)) {
                ++it;
                std::string operand_2_expr;
                if (op == adobe::or_k || op == adobe::and_k) {
                    const adobe::array_t& subexpr = (it++)->cast<adobe::array_t>();
                    adobe::array_t::const_reverse_iterator subexpr_it = subexpr.rbegin();
                    adobe::array_t::const_reverse_iterator subexpr_end_it = subexpr.rend();
                    WriteExpressionImpl(subexpr_it, subexpr_end_it, operand_2_expr);
                } else {
                    WriteExpressionImpl(it, end_it, operand_2_expr);
                }
                WriteExpressionImpl(it, end_it, output);
                output += BinaryOpString(op);
                output += operand_2_expr;
            } else if (UnaryOp(op)) {
                ++it;
                std::string operand_2_expr;
                WriteExpressionImpl(it, end_it, operand_2_expr);
                output += UnaryOpString(op);
                output += operand_2_expr;
            } else if (op == adobe::bracket_index_k || op == adobe::dot_index_k) {
                ++it;
                std::string operand_2_expr;
                WriteExpressionImpl(it, end_it, operand_2_expr);
                WriteExpressionImpl(it, end_it, output);
                if (op == adobe::bracket_index_k) {
                    if (output[output.size() - 1] == ' ')
                        output.resize(output.size() - 1);
                    output += '[';
                    output += operand_2_expr;
                    if (output[output.size() - 1] == ' ')
                        output.resize(output.size() - 1);
                    output += ']';
                } else {
                    if (output[output.size() - 1] == ' ')
                        output.resize(output.size() - 1);
                    output += '.';
                    output += operand_2_expr;
                }
            } else {
                WritePrimaryExpression(it, end_it, output);
            }
        } else {
            WritePrimaryExpression(it, end_it, output);
        }
    }

}

std::string GG::WriteExpression(const adobe::array_t& expression)
{
    std::string retval;
    adobe::array_t::const_reverse_iterator it = expression.rbegin();
    adobe::array_t::const_reverse_iterator end_it = expression.rend();
    WriteExpressionImpl(it, end_it, retval);
    return retval;
}
