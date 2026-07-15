cmake_minimum_required(VERSION 3.25)

project(CozyChorusSuite VERSION 0.1.0 LANGUAGES CXX C)

# --- Visual Studio Solution Explorer layout (paired with the block at EOF) ---
# Turn on Solution Explorer folders and rename CMake's own utility-project folder
# (ALL_BUILD / ZERO_CHECK / INSTALL) so every auto-generated project ends up in
# one collapsed bucket. These are global and must be set before targets exist.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Build (auto-generated)")

# JUCE's own option: collect its module sources under a tidy "JUCE Modules" IDE
# filter (headers marked header-only) instead of dumping them in "Source Files"
# next to ours. Must be in the cache before JUCE is added below.
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL "" FORCE)

# JUCE, pinned as a git submodule (JUCE/ @ 8.0.14).
add_subdirectory(JUCE)

# VST3 + Standalone everywhere; AU only builds on Apple platforms.
set(COZY_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND COZY_FORMATS AU)
endif()

juce_add_plugin(CozyChorusSuite
    COMPANY_NAME                "Seank23"
    BUNDLE_ID                   com.seank23.CozyChorusSuite
    PLUGIN_MANUFACTURER_CODE    Sk23
    PLUGIN_CODE                 Cczs
    FORMATS                     ${COZY_FORMATS}
    PRODUCT_NAME                "CozyChorus Suite"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             Fx Modulation
    AU_MAIN_TYPE                kAudioUnitType_Effect)

# CONFIGURE_DEPENDS re-runs the glob at build time, so new Source/ files are picked
# up without editing CMake. Feeds both target_sources and the source_group at EOF.
file(GLOB_RECURSE COZY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h")

target_sources(CozyChorusSuite PRIVATE ${COZY_SOURCES})

target_compile_features(CozyChorusSuite PRIVATE cxx_std_20)

target_include_directories(CozyChorusSuite
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_compile_definitions(CozyChorusSuite
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(CozyChorusSuite
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------
# Visual Studio solution organisation
#
# CMake regenerates the .slnx on every configure (and ZERO_CHECK re-runs CMake
# whenever this file changes), so solution layout has to be expressed here —
# hand-edits to the .slnx get overwritten. Goal: keep the one editable code
# target, CozyChorusSuite (everything under Source/), at the solution root and
# tuck every auto-generated wrapper/helper project into collapsed folders.
# ---------------------------------------------------------------------------

# Mirror the on-disk Source/ tree in the project instead of a flat file list,
# keeping our files in their own "Source" filter, separate from JUCE's modules.
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source" PREFIX "Source" FILES ${COZY_SOURCES})

# Hoist the editable code target to the solution root (JUCE otherwise nests it
# in a folder named after the target) so it is the most prominent project.
set_target_properties(CozyChorusSuite PROPERTIES FOLDER "")

# The format wrappers you actually build / run.
foreach(cozy_target CozyChorusSuite_Standalone CozyChorusSuite_VST3)
    if(TARGET ${cozy_target})
        set_target_properties(${cozy_target} PROPERTIES FOLDER "Formats")
    endif()
endforeach()

# Pure plumbing: the "build every format" aggregate plus JUCE's resource and
# VST3-manifest helper libs — hidden next to CMake's own utility projects.
foreach(cozy_target CozyChorusSuite_All CozyChorusSuite_rc_lib CozyChorusSuite_vst3_helper)
    if(TARGET ${cozy_target})
        set_target_properties(${cozy_target} PROPERTIES FOLDER "Build (auto-generated)")
    endif()
endforeach()

# Press F5 -> launch the standalone app (rather than CMake's ALL_BUILD default).
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    PROPERTY VS_STARTUP_PROJECT CozyChorusSuite_Standalone)

# ---------------------------------------------------------------------------
# Docs project
#
# A build-free utility target that just lists the Markdown docs so they are
# browsable/editable in Solution Explorer. add_custom_target(... SOURCES ...)
# registers files for the IDE only — .md has no compile step, and without the
# ALL keyword this target is never part of the default build. GLOB with
# CONFIGURE_DEPENDS re-scans at build time, so new docs appear on their own.
# ---------------------------------------------------------------------------
file(GLOB COZY_ROOT_DOCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.md")
file(GLOB COZY_SUB_DOCS  CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/docs/*.md")

add_custom_target(Docs SOURCES ${COZY_ROOT_DOCS} ${COZY_SUB_DOCS})

# Root docs (CLAUDE.md, DEVLOG.md, ...) sit at the project root; the docs/ folder
# gets its own filter mirroring the on-disk layout.
source_group("docs" FILES ${COZY_SUB_DOCS})