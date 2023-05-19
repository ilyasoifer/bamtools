// ***************************************************************************
// rewrite_read_groups.h 
// ---------------------------------------------------------------------------
// Last modified: 7 April 2011
// ---------------------------------------------------------------------------
// Renames read groups in the file
// ***************************************************************************

#ifndef REWRITE_READ_GROUPS_H
#define REWRITE_READ_GROUPS_H

#include <toolkit/bamtools_tool.h>

namespace BamTools {

class RewriteReadGroupsTool : public AbstractTool
{

public:
    RewriteReadGroupsTool();
    ~RewriteReadGroupsTool();

public:
    int Help();
    int Run(int argc, char* argv[]);

private:
    struct RewriteReadGroupsToolSettings;
    RewriteReadGroupsToolSettings* m_settings;

    struct RewriteReadGroupsToolPrivate;
    RewriteReadGroupsToolPrivate* m_impl;
};

}  // namespace BamTools

#endif  // REWRITE_READ_GROUPS_H
