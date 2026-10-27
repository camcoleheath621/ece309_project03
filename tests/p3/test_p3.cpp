// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples
// Test 1: Add and Find a Registered Tool
TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

// Test 2: Reject Duplicate Tools and Preserve the Original
TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

// Test 3: Reject Paths that Escape the Workspace
TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

// Test 4: Verify Calculator Operator Precedence
TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

// Test 5: Reject Division by Zero
TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

// Test 6: Register and Retrieve 10,000 Tools
TEST(reg_10000_tools) {
    ToolRegistry r;
    Tool* first = nullptr;

    // insert 10,000 uniquely named tools
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        auto tool = std::make_unique<NamedTool>(name);

        if (i == 0) {
            first = tool.get();                 // save first tool's address
        }

        REQUIRE(r.add(std::move(tool)));        // insert tool
        CHECK(r.load_factor() <= 1.0);          // verify load factor
    }

    CHECK(r.size() == 10000);                   // verify total count

    // verify every tool can be found
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        REQUIRE(r.find(name) != nullptr);
    }

    CHECK(r.find("tool_0") == first);            // verify pointer stability
    CHECK(r.max_probe_length() <= 64);          // verify probe limit

    // print registry measurements
    std::printf("Buckets: %zu\n", r.bucket_count());
    std::printf("Load factor: %.3f\n", r.load_factor());
    std::printf("Max probe length: %zu\n", r.max_probe_length());
}

// Test 7: Compare FNV-1a Against a Weaker Hash Function
TEST(reg_alternative_hash) {
    ToolRegistry r;

    // insert 10,000 tools using our registry
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        REQUIRE(r.add(std::make_unique<NamedTool>(name)));
    }

    // create counters for the weak hash
    std::vector<std::size_t> weak_buckets(r.bucket_count(), 0);

    // calculate bucket indices using the weak hash
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);

        std::size_t weak_hash =
            static_cast<unsigned char>(name[0]) + name.size();

        std::size_t index = weak_hash % weak_buckets.size();
        ++weak_buckets[index];
    }

    // find the longest weak-hash chain
    std::size_t longest_weak = 0;

    for (std::size_t count : weak_buckets) {
        if (count > longest_weak) {
            longest_weak = count;
        }
    }

    // print both hash measurements
    std::printf("FNV-1a max chain: %zu\n", r.max_probe_length());
    std::printf("Weak hash max chain: %zu\n", longest_weak);

    CHECK(longest_weak > r.max_probe_length());
}

// Test 8: Reject Null Tools
TEST(reg_null_rejected) {
    ToolRegistry r;

    CHECK(r.size() == 0);                // verify empty registry
    CHECK(!r.add(nullptr));              // reject null tool
    CHECK(r.size() == 0);                // verify count unchanged
    CHECK(r.find("unknown") == nullptr);  // verify missing tool
}

// Test 9: Visit Each Registered Tool Exactly Once
TEST(reg_for_each_visits_each_once) {
    ToolRegistry r;

    // register three tools
    REQUIRE(r.add(std::make_unique<NamedTool>("alpha")));
    REQUIRE(r.add(std::make_unique<NamedTool>("beta")));
    REQUIRE(r.add(std::make_unique<NamedTool>("gamma")));

    // initialize all to zero
    int visits = 0;
    int alpha = 0;
    int beta = 0;
    int gamma = 0;

    // visit every registered tool
    r.for_each([&](const Tool& tool) {
        ++visits;

        if (tool.name() == "alpha") {
            ++alpha;
        }
        else if (tool.name() == "beta") {
            ++beta;
        }
        else if (tool.name() == "gamma") {
            ++gamma;
        }
    });

    // verify each tool was visited exactly once
    CHECK(visits == 3);
    CHECK(alpha == 1);
    CHECK(beta == 1);
    CHECK(gamma == 1);
}

// Test 10: Subtraction evaluates left to right
TEST(calc_subtraction_left_to_right) {
    double result = 0;
    std::string error;

    REQUIRE(eval("10 - 4 - 3", &result, &error));
    CHECK(result == 3);
}

// Test 11: Division evaluates left to right
TEST(calc_division_left_to_right) {
    double result = 0;
    std::string error;

    REQUIRE(eval("100 / 10 / 5", &result, &error));
    CHECK(result == 2);
}

// Test 12: Parentheses override normal precedence
TEST(calc_parentheses) {
    double result = 0;
    std::string error;

    REQUIRE(eval("(2 + 3) * 4", &result, &error));
    CHECK(result == 20);
}

// Test 13: Multiple operations with parentheses
TEST(calc_multiple_operations) {
    double result = 0;
    std::string error;

    REQUIRE(eval("2 * (3 + 4) - 5", &result, &error));
    CHECK(result == 9);
}

// Test 14: Multiply two negative numbers
TEST(calc_negative_numbers) {
    double result = 0;
    std::string error;

    REQUIRE(eval("-3 * -3", &result, &error));
    CHECK(result == 9);
}

// Test 15: Division produces a decimal result
TEST(calc_decimal_result) {
    double result = 0;
    std::string error;

    REQUIRE(eval("7 / 2", &result, &error));
    CHECK(result == 3.5);
}

// Test 16: Reject division by zero inside parentheses
TEST(calc_division_by_zero_parentheses) {
    double result = 0;
    std::string error;

    CHECK(!eval("1 / (2 - 2)", &result, &error));
    CHECK(error == "division by zero");
}

// Test 17: Evaluate multiple operators without parentheses
TEST(calc_multiple_operators) {
    double result = 0;
    std::string error;

    REQUIRE(eval("8 + 6 / 3 * 2", &result, &error));
    CHECK(result == 12);
}

// Test 18: Evaluate decimal numbers with parentheses
TEST(calc_decimal_parentheses) {
    double result = 0;
    std::string error;

    REQUIRE(eval("(1.5 + 2.25) * 2", &result, &error));
    CHECK(result == 7.5);
}

// Test 19: Evaluate 20,000 addition terms
TEST(calc_large_expression) {
    double result = 0;
    std::string error;

    std::string expression = "1";

    // create an expression with 20,000 terms
    for (int i = 1; i < 20000; ++i) {
        expression += "+1";
    }

    REQUIRE(eval(expression, &result, &error));
    CHECK(result == 20000);
}

// Test 20: Evaluate 5,000 nested parentheses
TEST(calc_deep_parentheses) {
    double result = 0;
    std::string error;

    // create 5,000 opening parentheses
    std::string expression(5000, '(');

    expression += "7";              // value inside parentheses

    // add 5,000 closing parentheses
    expression.append(5000, ')');

    REQUIRE(eval(expression, &result, &error));
    CHECK(result == 7);
}

// Test 21: Accept and Normalize Valid Workspace Paths
TEST(ws_valid_paths) {
    TempDir d;
    Workspace ws(d.path());

    struct Case {
        std::string input;
        fs::path expected;
    };

    // define valid paths and expected results
    std::vector<Case> cases = {
        {"notes/todo.txt", ws.root() / "notes/todo.txt"},
        {"./notes//todo.txt", ws.root() / "notes/todo.txt"},
        {".", ws.root()},
        {"a/../b", ws.root() / "b"},
        {"a/..", ws.root()},
        {"new/dir/file.txt", ws.root() / "new/dir/file.txt"},
        {"my..notes.txt", ws.root() / "my..notes.txt"}
    };

    // verify each path resolves correctly
    for (const auto& test : cases) {
        fs::path out;
        std::string reason;

        REQUIRE(ws.resolve(test.input, &out, &reason));
        CHECK(out == test.expected);
    }
}

// Test 22: Reject Invalid Workspace Paths
TEST(ws_invalid_paths) {
    TempDir d;
    Workspace ws(d.path());

    // define invalid paths
    std::vector<std::string> paths = {
        "",
        std::string("notes") + '\0' + "/x",
        "/etc/passwd",
        "..",
        "../etc/passwd",
        "a/../../b",
        "notes/../../ws/notes/todo.txt"
    };

    // verify each invalid path is rejected
    for (const auto& path : paths) {
        fs::path out;
        std::string reason;

        CHECK(!ws.resolve(path, &out, &reason));
        CHECK(!reason.empty());
    }
}
// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives

int main(int argc, char** argv) { return mt::run_all(argc, argv); }
