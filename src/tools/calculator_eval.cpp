// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

// src/tools/calculator_eval.cpp

#include "tools/calculator.h"
#include <stack>
#include <string>

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {

    using Kind = CalcToken::Kind;

    std::stack<double> values;       // stores numbers and results
    std::stack<Kind> ops;            // stores operators

    // get operator priority
    auto precedence = [](Kind op) -> int {
        if (op == Kind::Times || op == Kind::Divide) {
            return 2;
        }
        if (op == Kind::Plus || op == Kind::Minus) {
            return 1;
        }
        return 0;
    };

    // apply the operator at the top of the stack
    auto apply = [&]() -> bool {

        // verify an operator and two numbers exist
        if (ops.empty() || values.size() < 2) {
            *error = "invalid expression";
            return false;
        }

        Kind op = ops.top();
        ops.pop();

        double right = values.top();  // get right operand
        values.pop();

        double left = values.top();   // get left operand
        values.pop();

        double result = 0.0;

        // perform the operation
        switch (op) {
            case Kind::Plus:
                result = left + right;
                break;

            case Kind::Minus:
                result = left - right;
                break;

            case Kind::Times:
                result = left * right;
                break;

            case Kind::Divide:
                if (right == 0.0) {
                    *error = "division by zero";
                    return false;
                }
                result = left / right;
                break;

            default:
                *error = "invalid operator";
                return false;
        }

        values.push(result);          // save calculated result
        return true;
    };

    // process each token
    for (const CalcToken& token : tokens) {

        Kind kind = token.kind;

        // store numbers
        if (kind == Kind::Number) {
            values.push(token.value);
        }

        // store opening parentheses
        else if (kind == Kind::LParen) {
            ops.push(kind);
        }

        // evaluate inside parentheses
        else if (kind == Kind::RParen) {

            while (!ops.empty() && ops.top() != Kind::LParen) {
                if (!apply()) {
                    return false;
                }
            }

            // verify opening parenthesis exists
            if (ops.empty()) {
                *error = "mismatched parentheses";
                return false;
            }

            ops.pop();                // remove opening parenthesis
        }

        // process mathematical operators
        else if (precedence(kind) > 0) {

            // evaluate operators with equal or higher priority
            while (!ops.empty() &&
                   ops.top() != Kind::LParen &&
                   precedence(ops.top()) >= precedence(kind)) {

                if (!apply()) {
                    return false;
                }
            }

            ops.push(kind);           // save current operator
        }

        else {
            *error = "invalid token";
            return false;
        }
    }

    // evaluate remaining operators
    while (!ops.empty()) {
        if (!apply()) {
            return false;
        }
    }

    // verify exactly one result remains
    if (values.size() != 1) {
        *error = "invalid expression";
        return false;
    }

    *out = values.top();              // store final result
    error->clear();
    return true;
}
