#include "query.hpp"
#include "component.hpp"
#include "world.hpp"
#include "logger.hpp"

void Query::compile(World* world) {
    LOG_DEBUG("Compiling query with ", requires.size(), " required and ", rejects.size(), " rejected components");
    
    required.reset();
    for (auto& type : requires) {
        int bit = world->assign(type);
        required.set(bit);
        LOG_DEBUG("Setting required bit ", bit, " for component type ", type.name());
    }
    
    rejected.reset();
    for (auto& type : rejects) {
        int bit = world->assign(type);
        rejected.set(bit);
        LOG_DEBUG("Setting rejected bit ", bit, " for component type ", type.name());
    }
}

bool Query::matches(ComponentMask mask) const {
    bool matches = ((mask & required) == required) && ((mask & rejected) == 0);
    LOG_DEBUG("Query match result: ", matches, 
              " (mask: ", mask.to_string(),
              ", required: ", required.to_string(),
              ", rejected: ", rejected.to_string(), ")");
    return matches;
}