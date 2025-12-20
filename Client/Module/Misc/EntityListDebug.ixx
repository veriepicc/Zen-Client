export module EntityListDebug;

import Module;
import Global;
import Actor;
import RuntimeIDComponent;
import Utils;
import <iostream>;
import <vector>;
import <string>;

export class EntityListDebug : public Module
{
public:
    EntityListDebug() : Module("EntityListDebug", "Verified entity list access.", Category::Misc, 0) 
    {
        Register();
    }

    void onUpdate() override
    {
        // Debug modules should be silent unless we need them for data verification.
        // We've verified the list works, so we'll stop the spam.
    }
};

export EntityListDebug entityListDebugInstance{};
