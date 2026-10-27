// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.


#include "tools/workspace.h"

#include <string>

namespace fs = std::filesystem;

// validate and resolve a workspace-relative path
bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {

    // reject empty paths
    if (user_path.empty()) {
        *reason = "empty path";
        return false;
    }

    // reject embedded null characters
    if (user_path.find('\0') != std::string_view::npos) {
        *reason = "path contains NUL";
        return false;
    }

    // convert input into a filesystem path
    fs::path p{std::string(user_path)};

    // reject absolute paths
    if (p.is_absolute() || p.has_root_path()) {
        *reason = "absolute path not allowed";
        return false;
    }

    // remove unnecessary path components
    fs::path normalized = p.lexically_normal();

    // reject paths that escape the workspace
    if (!normalized.empty() && *normalized.begin() == "..") {
        *reason = "path escapes workspace";
        return false;
    }

    // resolve current workspace directory
    if (normalized == ".") {
        *out = root();
    }
    else {
        *out = root() / normalized;
    }

    reason->clear();
    return true;
}

