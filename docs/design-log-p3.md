# Project 3 Design Log

**Author:** Camden Heath

**P2 code used:** My own Project 2 implementation.

**Tools used:** VS Code, WSL Ubuntu, CMake, Git, GitHub, and ChatGPT (GPT-6). I used ChatGPT to assist with program structure, debugging, testing, and developing the ToolRegistry, Workspace, and calculator implementations. I reviewed the implementations, verified their behavior through testing, and used Git to track my progress.


## 1. Hash Table
I implemented the ToolRegistry using separate chaining, where each bucket contains a vector of unique pointers to Tool objects. I chose this approach over open addressing because collisions can be handled more efficiently without having to search for another available position. This technique makes collision handling and resizing easier.

For the hash function, I used the 64-bit FNV-1a algorithm to process every character in a tool's name. The registry starts with 8 buckets and doubles its capacity whenever adding another tool would exceed a load factor of 0.75. Checking the load factor before insertion helps maintain an efficient distribution of tools.

When testing 10,000 uniquely named tools, the registry expanded to 16,384 buckets, with a load factor of around 0.610 and a maximum chain length of 6. In comparison, a weaker hash function using the first character and string length produced a maximum chain length of 9,000.

These results demonstrate how a stronger hash function distributes tools more evenly and reduces collisions. The average amortized time complexity of add() is O(1), since most insertions require only a bucket lookup and insertion. Although resizing takes O(n) time, it happens infrequently because the number of buckets doubles each time.

## 2. Pointer stability
In my ToolRegistry, each Tool object is dynamically allocated and owned by a `std::unique_ptr`. When the hash table resizes, the unique pointers are moved into new buckets. The actual Tool objects remain at their original memory addresses. Because of this, a `Tool*` returned by `find()` remains valid even after resizing, as long as the registry still owns that tool.

## 3. Calculator trace
For the expression `2 * (3 + 4) - 5`, I used two stacks: `values` to store numbers and intermediate results, and `ops` to store operators and parentheses. Both stacks begin empty.

First, `2` is pushed onto `values`, followed by `*` and `(` being pushed onto `ops`. Next, `3` is pushed onto `values`, `+` onto `ops`, and `4` onto `values`.

When the closing parenthesis `)` is encountered, the calculator evaluates `3 + 4`, producing `7`, and removes the opening parenthesis. At this point, `values = [2, 7]` and `ops = [*]`.

The next operator, `-`, has lower precedence than `*`, so multiplication is performed first, producing `14`. The subtraction operator is then pushed onto `ops`, followed by `5` onto `values`.

Finally, the remaining subtraction is performed as `14 - 5`, leaving `9` as the final result.

The following table below shows the contents of both stacks after processing each token, with the rightmost item representing the top of each stack.

| Token | Values Stack | Operators Stack |
|-------|--------------|-----------------|
| Start | `[]` | `[]` |
| `2` | `[2]` | `[]` |
| `*` | `[2]` | `[*]` |
| `(` | `[2]` | `[*, (]` |
| `3` | `[2, 3]` | `[*, (]` |
| `+` | `[2, 3]` | `[*, (, +]` |
| `4` | `[2, 3, 4]` | `[*, (, +]` |
| `)` | `[2, 7]` | `[*]` |
| `-` | `[14]` | `[-]` |
| `5` | `[14, 5]` | `[-]` |
| End | `[9]` | `[]` |

## 4. Errors as values
I used return values to handle expected errors because situations such as missing files, invalid paths, or division by zero can occur during normal tool execution. These errors should not cause the entire program to stop.

Tools use `ToolResult::failure()` to return an error message, while `Workspace::resolve()` and `calc_evaluate()` return `false` and provide an explanation. This allows the harness to communicate the error to the model so it can adjust its request and continue.

Exceptions are more appropriate for unexpected problems, such as internal programming errors or situations where the program cannot safely continue.

## 5. What I would design differently
If I redesigned the ToolRegistry, I would improve how it handles resizing. Currently, the registry starts with 8 buckets and doubles its capacity whenever the load factor threshold is reached. While this approach works, each resize requires redistributing all existing entries, which takes O(n) time. I would also consider allowing an estimated number of tools to be specified beforehand so the registry could allocate enough buckets initially. This could reduce unnecessary resizing.