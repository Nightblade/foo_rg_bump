#include "stdafx.h"
#include "guids.h"

#include <cstdio>
#include <cstdlib>
#include <cmath>

DECLARE_COMPONENT_VERSION(
    "ReplayGain Track Gain Adjuster",
    "1.0",
    "Adjusts REPLAYGAIN_TRACK_GAIN of the currently playing track by +/- 0.5 dB.\n"
    "Assign keyboard shortcuts via Preferences > Keyboard Shortcuts.\n\n"
    "Commands:\n"
    "  RG Adjust / Track Gain +0.5 dB\n"
    "  RG Adjust / Track Gain -0.5 dB"
);

VALIDATE_COMPONENT_FILENAME("foo_rg_bump.dll");

// ---------------------------------------------------------------------------
// Tag name we operate on
// ---------------------------------------------------------------------------
static const char k_tag_name[] = "REPLAYGAIN_TRACK_GAIN";
static const double k_delta    = 0.5;

// ---------------------------------------------------------------------------
// file_info_filter implementation
//
// update_info_async calls our filter once per track with a mutable file_info.
// We read the current gain value, apply the delta, and write it back.
// ---------------------------------------------------------------------------
class rg_gain_filter : public file_info_filter
{
public:
    explicit rg_gain_filter(double delta) : m_delta(delta) {}

    // Returns true if we actually modified the info (tells fb2k to write the file).
    bool apply_filter(metadb_handle_ptr /*handle*/, t_filestats /*stats*/, file_info & info) override
    {
        // Read current value; default to 0.0 dB if absent or unparseable.
        double current = 0.0;
        const char * existing = info.meta_get(k_tag_name, 0);
        if (existing && *existing)
        {
            // The tag format is e.g. "-3.20 dB" or just "-3.20".
            // strtod stops at the first non-numeric char, so both forms work.
            char * end = nullptr;
            double parsed = std::strtod(existing, &end);
            if (end != existing)
                current = parsed;
        }

        double adjusted = current + m_delta;

        // Format as "+N.NN dB" or "-N.NN dB", matching the conventional RG format.
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%+.2f dB", adjusted);

        info.meta_set(k_tag_name, buf);
        return true;
    }

private:
    double m_delta;
};

// ---------------------------------------------------------------------------
// Helper: get now-playing handle and schedule a tag update
// ---------------------------------------------------------------------------
static void adjust_gain(double delta)
{
    // playback_control methods must be called from the main thread only.
    // mainmenu_commands::execute() is guaranteed to run on the main thread.
    auto pc = playback_control::get();
    metadb_handle_ptr track;
    if (!pc->get_now_playing(track))
        return; // nothing playing, silently do nothing

    metadb_handle_list tracks;
    tracks.add_item(track);

    auto filter = fb2k::service_new<rg_gain_filter>(delta);

    // update_info_async writes tags to the physical file asynchronously.
    // The second parameter is an optional progress dialog parent (NULL = none).
    metadb_io_v2::get()->update_info_async(
        tracks,
        filter,
        core_api::get_main_window(),
        0,
        nullptr
    );
}

// ---------------------------------------------------------------------------
// mainmenu_commands registration
//
// Registering commands here makes them appear in Preferences > Keyboard
// Shortcuts so the user can bind any key they like to each command.
// ---------------------------------------------------------------------------
class rg_mainmenu_commands : public mainmenu_commands
{
public:
    enum { cmd_gain_up = 0, cmd_gain_down = 1, cmd_count = 2 };

    t_uint32 get_command_count() override { return cmd_count; }

    GUID get_command(t_uint32 index) override
    {
        switch (index)
        {
        case cmd_gain_up:   return guid_cmd_gain_up;
        case cmd_gain_down: return guid_cmd_gain_down;
        default: uBugCheck();
        }
    }

    void get_name(t_uint32 index, pfc::string_base & out) override
    {
        switch (index)
        {
        case cmd_gain_up:   out = "Track Gain +0.5 dB"; break;
        case cmd_gain_down: out = "Track Gain -0.5 dB"; break;
        default: uBugCheck();
        }
    }

    bool get_description(t_uint32 index, pfc::string_base & out) override
    {
        switch (index)
        {
        case cmd_gain_up:
            out = "Increase REPLAYGAIN_TRACK_GAIN of the now-playing track by 0.5 dB";
            return true;
        case cmd_gain_down:
            out = "Decrease REPLAYGAIN_TRACK_GAIN of the now-playing track by 0.5 dB";
            return true;
        default: uBugCheck();
        }
    }

    GUID get_parent() override { return guid_mainmenu_group; }

    void execute(t_uint32 index, service_ptr_t<service_base> /*callback*/) override
    {
        switch (index)
        {
        case cmd_gain_up:   adjust_gain(+k_delta); break;
        case cmd_gain_down: adjust_gain(-k_delta); break;
        default: uBugCheck();
        }
    }
};

// ---------------------------------------------------------------------------
// mainmenu_group registration
//
// Creates the "RG Adjust" group under the View menu.
// Commands in this group appear in the keyboard shortcuts list as
// "RG Adjust / Track Gain +0.5 dB" etc.
// ---------------------------------------------------------------------------
static mainmenu_group_popup_factory g_mainmenu_group(
    guid_mainmenu_group,
    mainmenu_groups::view,          // parent: View menu
    mainmenu_commands::sort_priority_dontcare,
    "RG Adjust"
);

static mainmenu_commands_factory_t<rg_mainmenu_commands> g_mainmenu_commands_factory;
