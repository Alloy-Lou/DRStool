#ifdef _WIN32
    #include <direct.h>
    #define MAKE_DIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #define MAKE_DIR(path) mkdir(path, 0777)
#endif

#include <stdio.h>
#include <string.h>
#include "tinyfiledialogs.h"

int main() {
    printf("Starting :p ...\n");

    char ssf_path[1024];
    char audio_path[1024];
    char final_level_id[16];

    // Sauce: https://stackoverflow.com/questions/6145910/cross-platform-native-open-save-file-dialogs/47651444#47651444

    // Chart file
    char const * SSF_Filter[1] = { "*.ssf"}; 
    char const * ssf_file = tinyfd_openFileDialog(
        "Select the chart file",        // Title
        "",                             // Default path (leave empty for OS default)
        1,                              // Number of filter patterns
        SSF_Filter,                     // The patterns array
        "SSF File (use CHED-SSF)",      // Filter description in the dropdown
        0                               // Allow multiple selects? (0 = no, 1 = yes)
    );
    strcpy(ssf_path, ssf_file);
    // Audio file
    char const * audio_Filter[2] = { "*.mp3", "*.wav"}; 
    char const * audio_file = tinyfd_openFileDialog(
        "Select the main audio file",   // Title
        "",                             // Default path (leave empty for OS default)
        2,                              // Number of filter patterns
        audio_Filter,                     // The patterns array
        "Audio file",                   // Filter description in the dropdown
        0                               // Allow multiple selects? (0 = no, 1 = yes)
    );
    strcpy(audio_path, audio_file);
    // chart ID
    char const * level_id = tinyfd_inputBox(
        "Level ID", 
        "Enter the Level ID (numbers only, 5-digit format). Make sure you are not using an existing id (check music-info-base.xml)", 
        "10001" // Last known song id afaik is 00464, lessons is 90020 so 10xxx for customs ig?
    );
    strcpy(final_level_id, level_id);

    if (!ssf_file) {
        printf("No chart file selected :0 Exiting.\n");
        return 0;
    } else if (!audio_file) {
        printf("No audio file selected :0 Exiting.\n");
        return 0;
    } else if (!level_id) {
        printf("No Level ID provided. Exiting.\n");
        return 0;
    } else {
        printf("Selected chart file: %s\n", ssf_path);
        printf("Selected audio file: %s\n", audio_path);
        printf("Choosen id: %s\n", final_level_id);
    }

    // u sure?
    int start_processing = tinyfd_messageBox(
        "Start Processing?",
        "Are you readyyyy?",
        "yesno",     // Dialog type
        "question",  // Icon type
        1            // Default button (1 = yes)
    );

    if (start_processing == 1) {
        printf("\nProcessing started!\n");
        // TODO: do the actual stuff
    } else {
        printf("\nOperation cancelled :(\n");
    }

    return 0;
}