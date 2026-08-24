#include "stdafx.h"
#include "guids.h"
#include "version.h"

DECLARE_COMPONENT_VERSION(
    "ReplayGain Track Gain Adjuster",
    COMPONENT_VERSION,
    COMPONENT_FILE_NAME " " COMPONENT_VERSION "\n"
    "Copyright (c) " COMPONENT_COPYRIGHT ". All rights reserved.\n\n"
    "Adjusts REPLAYGAIN_TRACK_GAIN and/or REPLAYGAIN_ALBUM_GAIN of all selected playlist items "
    "and the currently playing track, by a configurable step size.\n\n"
    "Assign keyboard shortcuts via Preferences > Keyboard Shortcuts.\n\n"
    "Commands:\n"
    "  RG Adjust / Track Gain +delta dB\n"
    "  RG Adjust / Track Gain -delta dB\n\n"
    "Configure step size and target tag via Preferences > Advanced > Tools > RG Bump.\n\n"
    "Built with foobar2000 SDK 20241203\n"
    "on " __DATE__ " " __TIME__ "."
);

VALIDATE_COMPONENT_FILENAME("foo_rg_bump.dll");

// ---------------------------------------------------------------------------
// Advanced Preferences
//
// Preferences > Advanced > Tools > RG Bump
//
// Step size is stored as integer tenths of a dB (e.g. 5 = 0.5 dB).
// Target: 0 = track, 1 = album, 2 = both.
// ---------------------------------------------------------------------------
static advconfig_branch_factory g_advconfig_branch(
    "RG Bump", guid_advconfig_branch, advconfig_branch::guid_branch_tools, 0
);

static advconfig_integer_factory g_advconfig_delta(
    "Step size (tenths of a dB, e.g. 5 = 0.5 dB)",
    guid_advconfig_delta, guid_advconfig_branch,
    0,   // priority
    5,   // default: 0.5 dB
    1,   // minimum: 0.1 dB
    200  // maximum: 20.0 dB
);

static advconfig_integer_factory g_advconfig_target(
    "Target tag (0 = track, 1 = album, 2 = both)",
    guid_advconfig_target, guid_advconfig_branch,
    1,  // priority
    0,  // default: track
    0,  // minimum
    2   // maximum
);

// ---------------------------------------------------------------------------
// file_info_filter implementation
// ---------------------------------------------------------------------------
class rg_gain_filter : public file_info_filter
{
public:
    rg_gain_filter(double delta, int target) : m_delta(delta), m_target(target) {}

    bool apply_filter(metadb_handle_ptr /*handle*/, t_filestats /*stats*/, file_info & info) override
    {
        replaygain_info rg = info.get_replaygain();

        if (m_target == 0 || m_target == 2)
        {
            float current = rg.m_track_gain;
            if (current == replaygain_info::gain_invalid)
                current = 0.0f;
            rg.m_track_gain = pfc::clip_t<float>(current + (float)m_delta, -51.0f, +51.0f);
        }

        if (m_target == 1 || m_target == 2)
        {
            float current = rg.m_album_gain;
            if (current == replaygain_info::gain_invalid)
                current = 0.0f;
            rg.m_album_gain = pfc::clip_t<float>(current + (float)m_delta, -51.0f, +51.0f);
        }

        info.set_replaygain(rg);
        return true;
    }

private:
    double m_delta;
    int    m_target;
};

// ---------------------------------------------------------------------------
// Helper: resolve the target track and schedule a tag update
// ---------------------------------------------------------------------------
static void adjust_gain(double delta)
{
    metadb_handle_list tracks;

    metadb_handle_ptr playing;
    if (playback_control::get()->get_now_playing(playing))
    {
        tracks.add_item(playing);
    }

    metadb_handle_list selected;
    auto plm = playlist_manager::get();
    t_size playlist = plm->get_active_playlist();
    if (playlist != pfc::infinite_size)
    {
        plm->playlist_get_selected_items(playlist, selected);
        tracks.add_items(selected);
    }

    if (tracks.get_count() == 0) return;

    tracks.sort_remove_duplicates_t([](const metadb_handle_ptr & a, const metadb_handle_ptr & b)
    {
        return metadb::path_compare_metadb_handle(a, b) < 0;
    });

    int target = (int)(uint64_t)g_advconfig_target;
    auto filter = fb2k::service_new<rg_gain_filter>(delta, target);

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
        case cmd_gain_up:   out = "Track Gain +delta dB"; break;
        case cmd_gain_down: out = "Track Gain -delta dB"; break;
        default: uBugCheck();
        }
    }

    bool get_description(t_uint32 index, pfc::string_base & out) override
    {
        switch (index)
        {
        case cmd_gain_up:
            out = "Increase ReplayGain value(s) of selected and playing tracks by the configured step size";
            return true;
        case cmd_gain_down:
            out = "Decrease ReplayGain value(s) of selected and playing tracks by the configured step size";
            return true;
        default: uBugCheck();
        }
    }

    GUID get_parent() override { return guid_mainmenu_group; }

    void execute(t_uint32 index, service_ptr_t<service_base> /*callback*/) override
    {
        double delta = (double)(uint64_t)g_advconfig_delta / 10.0;
        switch (index)
        {
        case cmd_gain_up:   adjust_gain(+delta); break;
        case cmd_gain_down: adjust_gain(-delta); break;
        default: uBugCheck();
        }
    }
};

// ---------------------------------------------------------------------------
// mainmenu_group registration
// ---------------------------------------------------------------------------
static mainmenu_group_popup_factory g_mainmenu_group(
    guid_mainmenu_group,
    mainmenu_groups::view,
    mainmenu_commands::sort_priority_dontcare,
    "RG Adjust"
);

static mainmenu_commands_factory_t<rg_mainmenu_commands> g_mainmenu_commands_factory;
