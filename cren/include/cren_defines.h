#ifndef CREN_DEFINES_INCLUDED
#define CREN_DEFINES_INCLUDED

/// @brief platform detection
#ifdef _WIN32
    #define PLATFORM_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #define PLATFORM_APPLE
#elif defined(__ANDROID__)
    #define PLATFORM_ANDROID
#elif defined(__linux__)
	#if defined(__WAYLAND__)
	    #define PLATFORM_WAYLAND
	#elif defined(__X11__)
	    #define PLATFORM_X11
	#endif
#else
    #error "Unsupported platform"
#endif

/// @brief building as DLL or static-lib, depending on context
#ifdef CREN_SHARED_LIBRARY
    #ifdef PLATFORM_WINDOWS
        #ifdef CREN_BUILDING_DLL
            #define CREN_API __declspec(dllexport)
        #else
            #define CREN_API __declspec(dllimport)
        #endif
    #else
        #define CREN_API __attribute__((visibility("default")))
    #endif
#else
    #define CREN_API
#endif

/// @brief  align-as per compiler
#if defined(_MSC_VER)
    #define align_as(X) __declspec(align(X))
#elif defined(__GNUC__) || defined(__clang__)
    #define align_as(X) __attribute__((aligned(X)))
#else
    #include <stdalign.h>
    #define align_as(X) _Alignas(X)
#endif

/// @brief returns an unsigned integer of a version, 0.1.2.325 as an example
#define CREN_MAKE_VERSION(variant, major, minor, patch) ((((unsigned int)(variant)) << 29U) | (((unsigned int)(major)) << 22U) | (((unsigned int)(minor)) << 12U) | ((unsigned int)(patch)))

/// @brief size of a static C-style array. don't use on pointers
#define CREN_ARRAYSIZE(ARR) ((int)(sizeof(ARR) / sizeof(*(ARR))))     

/// @brief how many frames are simultaneosly rendered (multi-buffering)
#define CREN_CONCURRENTLY_RENDERED_FRAMES 2

/// @brief how many characters a path may have
#define CREN_PATH_MAX_SIZE 128

/// @brief How many descriptors sets at max a layout binding may have
#define CREN_PIPELINE_DESCRIPTOR_SET_LAYOUT_BINDING_MAX 32

/// @brief How many push constants at max may exist for a given Pipeline
#define CREN_PIPELINE_PUSH_CONSTANTS_MAX 8

/// @brief How many shader stages a pipeline may have, since we only support Vertex and Fragment for now, 2
#define CREN_PIPELINE_SHADER_STAGES_COUNT 2

/// @brief The quad's default pipeline name, used for hashtable look-ups
#define CREN_PIPELINE_QUAD_DEFAULT_NAME "Quad:Default"

/// @brief The quad's picking pipeline name, used for hashtable look-ups
#define CREN_PIPELINE_QUAD_PICKING_NAME "Quad:Picking"

#endif // CREN_DEFINES_INCLUDED