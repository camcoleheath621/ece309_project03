#include "core/sentinel_scanner.h"

//copy the sentinel parameter into the memember variable sentinel_
SentinelScanner::SentinelScanner(std::string sentinel) {
    sentinel_ = sentinel;
}

SentinelScanner::Out SentinelScanner::feed(std::string_view chunk) {
    
    std::string combined = pending_ + std::string(chunk);  // combine held-back text w/ new chunk
    
    std::size_t pos = combined.find(sentinel_);             // find where the full sentinel begins

    if (pos != std::string::npos) {                          // if the full sentinel was found
        Out result;                                         // create return type variable
        
        result.safe_text = combined.substr(0,pos);          // keep only the text before the sentinel
        result.sentinel_found = true;                       // record the sentinel that was found
        pending_.clear();                                   // clear the chunks in pending_
        return result;      // return safe text and true
    }

    else {
        Out result;

        std::size_t keep = sentinel_.size()-1;
        
        if (keep > combined.size()) {
            keep = combined.size();
        }

        pending_ = combined.substr(combined.size() - keep);

        result.safe_text = combined.substr(0, combined.size() - keep);
        result.sentinel_found = false;
        
        return result;      // return safe and false
    }


}

SentinelScanner::Out SentinelScanner::flush() {
    
    Out result;                     

    result.safe_text = pending_;       // release any text still being held
    result.sentinel_found = false;     // no full sentinel was found

    pending_.clear();                   // empty pending_ after releasing it

    return result;      // return safe and false


}