#ifdef _WIN32
    #include <direct.h>
    #define MAKE_DIR(path) _mkdir(path)
    #define CHANGE_DIR(path) _chdir(path)
#else
    #include <sys/stat.h>
    #include <unistd.h> // Needed for chdir on Linux
    #define MAKE_DIR(path) mkdir(path, 0777)
    #define CHANGE_DIR(path) chdir(path)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tinyfiledialogs.h"

int copy_file(const char *src_path, const char *dest_path) {
    FILE *src = fopen(src_path, "rb"); // Read Binary
    if (!src) return 0;

    FILE *dest = fopen(dest_path, "wb"); // Write Binary
    if (!dest) {
        fclose(src);
        return 0;
    }

    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes_read, dest);
    }

    fclose(src);
    fclose(dest);
    return 1; // Success
}

int main() {
    // Sauce: https://stackoverflow.com/questions/6145910/cross-platform-native-open-save-file-dialogs/47651444#47651444
    printf("Starting :p ...\n");
    freopen("log.log", "w", stdout);
    freopen("log.log", "a", stderr);
    setvbuf(stdout, NULL, _IONBF, 0); // actually write stuff asap and not wait the last moment u adhd language >:(
    // note to self, drs-converter seems to be messing with the logfile for some reason, too lazy to fix

    char ssf_path[1024];
    char audio_path[1024];
    char audio_preview_path[1024];
    char final_level_id[16];

    // do we have a save file :o ?

    FILE* settings = fopen("save.txt", "r");
    if (settings != NULL) {
        fgets(ssf_path, sizeof(ssf_path), settings);
        fgets(final_level_id, sizeof(final_level_id), settings);
        fclose(settings);

        // yeet the \n and \r out of the way or our file paths will be broken
        ssf_path[strcspn(ssf_path, "\r\n")] = 0;
        final_level_id[strcspn(final_level_id, "\r\n")] = 0;

        FILE* chart_file_exists = fopen(ssf_path, "r");
        if (chart_file_exists != NULL && (strlen(ssf_path) > 0 && strlen(audio_path) > 0)) {
            fclose(chart_file_exists);
            char prompt[2048];
            snprintf(prompt, sizeof(prompt), 
                "Do you want to quickly convert the last chart file converted into xml files?\n\n"
                "Last chart file: %s\nLevel ID: %s", 
                ssf_path, final_level_id
            );

            int use_saved = tinyfd_messageBox("Quick Run?", prompt, "yesno", "question", 1);
            if (use_saved == 1) {
                printf("Creating folder: %s\n", final_level_id);
                MAKE_DIR(final_level_id);
                if (!copy_file(ssf_path, "tools/drs-converter/test.ssf")) {
                    printf("ERROR: Failed to copy the chart file\n");
                    tinyfd_messageBox("Error", "Failed to copy the chart file.\n"
                        "Make sure the chart file and the level folder still exist", "ok", "error", 0);
                    return 1;
                }
                printf("Copied chart file to tools\n");
                printf("Navigating to drs-converter folder...\n");
                        
                CHANGE_DIR("tools/drs-converter");
                printf("Executing 'cargo run'...\n");
                int cargo_status = system("cargo run");
                CHANGE_DIR("../../");

                if (cargo_status != 0) {
                    printf("ERROR: cargo run failed or crashed.\n");
                    tinyfd_messageBox("Error", "drs-converter failed to run!", "ok", "error", 0);
                    return 1;
                }

                printf("drs-converter ran successfully\n");
                printf("Moving 'n renaming output\n");

                char dest_path[512];
                snprintf(dest_path, sizeof(dest_path), "%s/%s_1a.xml", final_level_id, final_level_id);
                copy_file("tools/drs-converter/output.xml", dest_path);
                snprintf(dest_path, sizeof(dest_path), "%s/%s_1b.xml", final_level_id, final_level_id);
                copy_file("tools/drs-converter/output.xml", dest_path);
                snprintf(dest_path, sizeof(dest_path), "%s/%s_2a.xml", final_level_id, final_level_id);
                copy_file("tools/drs-converter/output.xml", dest_path);
                snprintf(dest_path, sizeof(dest_path), "%s/%s_2b.xml", final_level_id, final_level_id);
                copy_file("tools/drs-converter/output.xml", dest_path);

                remove("tools/drs-converter/output.xml");
                remove("tools/drs-converter/test.ssf");

                tinyfd_messageBox(
                    "Done",
                    "Finished",
                    "ok",
                    "info",
                    1
                );
                return 0;
            }
        }
    }

    int processing_mode = tinyfd_messageBox(
        "Select files to convert",
        "Do you want to also convert audio files?\n"
        "Press YES to convert a chart file, an audio file and an audio preview file\n"
        "Press NO to only convert a chart file",
        "yesnocancel",      // Dialog type
        "question",         // Icon type
        1                   // Default button (1 = yes)
    );

    if (processing_mode == 0) {
        printf("\nOperation cancelled :(\n");
        return 1;
    }

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
    if (processing_mode == 1) {
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
        char const * audio_preview_file = tinyfd_openFileDialog(
            "Select the audio preview file",   // Title
            "",                             // Default path (leave empty for OS default)
            2,                              // Number of filter patterns
            audio_Filter,                     // The patterns array
            "Audio file",                   // Filter description in the dropdown
            0                               // Allow multiple selects? (0 = no, 1 = yes)
        );
        strcpy(audio_preview_path, audio_preview_file);
        if (!audio_file) {
            printf("No audio file selected :0 Exiting.\n");
            return 0;
        } else if (!audio_preview_file) {
            printf("No audio preview file selected :0 Exiting.\n");
            return 0;
        }
    }

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
        "Start converting files?",
        "yesno",     // Dialog type
        "question",  // Icon type
        1            // Default button (1 = yes)
    );

    if (start_processing != 1) {
        printf("\nOperation cancelled :(\n");
        return 0;
    }

    printf("\nProcessing started!\n");     
    printf("Creating folder: %s\n", final_level_id);
    MAKE_DIR(final_level_id);

    if (!copy_file(ssf_path, "tools/drs-converter/test.ssf")) {
        printf("ERROR: Failed to copy the chart file\n");
        tinyfd_messageBox("Error", "Failed to copy the chart file", "ok", "error", 0);
        return 1;
    }
    printf("Copied chart file to tools\n");
    printf("Navigating to drs-converter folder...\n");
            
    CHANGE_DIR("tools/drs-converter");
    printf("Executing 'cargo run'...\n");
    int cargo_status = system("cargo run");
    CHANGE_DIR("../../");

    if (cargo_status != 0) {
        printf("ERROR: cargo run failed or crashed.\n");
        tinyfd_messageBox("Error", "drs-converter failed to run!", "ok", "error", 0);
        return 1;
    }

    printf("drs-converter ran successfully\n");
    printf("Moving 'n renaming output\n");

    char dest_path[512];
    snprintf(dest_path, sizeof(dest_path), "%s/%s_1a.xml", final_level_id, final_level_id);
    copy_file("tools/drs-converter/output.xml", dest_path);
    snprintf(dest_path, sizeof(dest_path), "%s/%s_1b.xml", final_level_id, final_level_id);
    copy_file("tools/drs-converter/output.xml", dest_path);
    snprintf(dest_path, sizeof(dest_path), "%s/%s_2a.xml", final_level_id, final_level_id);
    copy_file("tools/drs-converter/output.xml", dest_path);
    snprintf(dest_path, sizeof(dest_path), "%s/%s_2b.xml", final_level_id, final_level_id);
    copy_file("tools/drs-converter/output.xml", dest_path);

    remove("tools/drs-converter/output.xml");
    remove("tools/drs-converter/test.ssf");

    if (processing_mode == 1) {
        printf("Copying audio files...\n");
        const char *audio_filename = audio_path; 
        const char *audio_preview_filename = audio_preview_path; 
        const char *last_slash = strrchr(audio_path, '/');
        const char *last_backslash = strrchr(audio_path, '\\');
        if (last_slash != NULL || last_backslash != NULL) {
            if (last_slash > last_backslash) {
                audio_filename = last_slash + 1; // skip the /
            } else {
                audio_filename = last_backslash + 1; 
            }
        }
        if (last_slash != NULL || last_backslash != NULL) {
            if (last_slash > last_backslash) {
                audio_preview_filename = last_slash + 1; // skip the /
            } else {
                audio_preview_filename = last_backslash + 1; 
            }
        }
        snprintf(dest_path, sizeof(dest_path), "tools/s3p_extract/%s", audio_filename);
        if (!copy_file(audio_path, dest_path)) {
            printf("ERROR: Failed to copy the audio file\n");
            tinyfd_messageBox("Error", "Failed to copy the audio file", "ok", "error", 0);
            return 1;
        }
        snprintf(dest_path, sizeof(dest_path), "tools/s3p_extract/%s", audio_preview_filename);
        if (!copy_file(audio_preview_path, dest_path)) {
            printf("ERROR: Failed to copy the audio_preview file\n");
            tinyfd_messageBox("Error", "Failed to copy the audio_preview file", "ok", "error", 0);
            return 1;
        }

        printf("Copied audio files to tools\n");
        printf("Navigating to s3p_extract folder...\n");
        
        printf("Searching for existing s3p_extract executable\n");

        #ifdef _WIN32
            const char *s3p_target = "tools/s3p_extract/s3p_extract.exe";
            const char *s3p_fallback = "bin/s3p_extract.exe";
        #else
            const char *s3p_target = "tools/s3p_extract/s3p_extract";
            const char *s3p_fallback = "bin/s3p_extract"; //if i ever build it on linux i'll include it and it'll work but rn meh
        #endif

        FILE *check_s3p = fopen(s3p_target, "r");
        if (check_s3p != NULL) {
            printf("Found user-compiled s3p_extract! Good boi, using that one.\n");
            fclose(check_s3p);
        } else {
            printf("s3p_extract not found in tools. But haveth not the worriey becaus' i happen to haveth on for ya :p\nCopying pre-built version from bin/...\n");

            int use_prebuilt = tinyfd_messageBox(
                "Tool Not Built",
                "No compiled s3p_extract executable was found in the tools folder.\n\n"
                "Click OK to use a pre-built version, or click Cancel to stop so you can compile it yourself (you will need a C compiler installed)\n\n"
                "Note: if you are running this tool on linux, the pre-built version will NOT work, you have to build it yourself.",
                "okcancel",
                "info",     
                1
            );

            if (use_prebuilt != 1) {
                printf("\nOperation cancelled by user to build the tool manually.\n");
                return 0;
            }

            if (!copy_file(s3p_fallback, s3p_target)) {
                printf("ERROR: Could not find or copy the pre-built s3p_extract from %s!\n", s3p_fallback);
                tinyfd_messageBox("Error", "Could not find or copy the pre-built s3p_extract from the bin folder!", "ok", "error", 0);
                return 1;
            }
        }

        printf("Executing s3p_extract...\n");
        CHANGE_DIR("tools/s3p_extract");
        char s3p_command[512];

        snprintf(s3p_command, sizeof(s3p_command), "s3p_extract -pack %s", audio_filename);
        int s3p_status = system(s3p_command);
        CHANGE_DIR("../../");
        if (s3p_status != 0) {
            printf("ERROR: s3p_extract failed or crashed.\n");
            tinyfd_messageBox("Error", "s3p_extract failed to run!", "ok", "error", 0);
            return 1;
        }

        printf("s3p_extract ran successfully\n");
        printf("Moving 'n renaming output\n");

        snprintf(dest_path, sizeof(dest_path), "%s/%s.s3p", final_level_id, final_level_id);
        copy_file("tools/s3p_extract/out.s3p", dest_path);
        remove("tools/s3p_extract/out.s3p");
        snprintf(dest_path, sizeof(dest_path), "tools/s3p_extract/%s", audio_filename);
        remove(dest_path);

        CHANGE_DIR("tools/s3p_extract");
        snprintf(s3p_command, sizeof(s3p_command), "s3p_extract -pack %s", audio_preview_filename);
        s3p_status = system(s3p_command);
        CHANGE_DIR("../../");
        if (s3p_status != 0) {
            printf("ERROR: s3p_extract failed or crashed.\n");
            tinyfd_messageBox("Error", "s3p_extract failed to run!", "ok", "error", 0);
            return 1;
        }

        printf("s3p_extract ran successfully\n");
        printf("Moving 'n renaming output\n");

        snprintf(dest_path, sizeof(dest_path), "%s/%spre.s3p", final_level_id, final_level_id);
        copy_file("tools/s3p_extract/out.s3p", dest_path);
        remove("tools/s3p_extract/out.s3p");
        snprintf(dest_path, sizeof(dest_path), "tools/s3p_extract/%s", audio_preview_filename);
        remove(dest_path);
    }

    const char *dummies[][2] = {
        {"dummy files/at_xxxxx.png",     "at_%s.png"},
        {"dummy files/jk_xxxxx_b.png",   "jk_%s_b.png"},
        {"dummy files/jk_xxxxx_m.png",   "jk_%s_m.png"},
        {"dummy files/jk_xxxxx_s.png",   "jk_%s_s.png"},
        {"dummy files/lc_xxxxx.png",     "lc_%s.png"},
        {"dummy files/ttat_xxxxx_b.png", "ttat_%s_b.png"},
        {"dummy files/ttat_xxxxx_s.png", "ttat_%s_s.png"},
        {"dummy files/tt_xxxxx.png",     "tt_%s.png"}
    };

    if (processing_mode == 1) {
        int copy_dummies = tinyfd_messageBox(
        "Copy dummy files?",
        "Would you also like to copy png dummmy files?",
        "yesno",     // Dialog type
        "question",  // Icon type
        1            // Default button (1 = yes)
        );
        if (copy_dummies == 0) {
            printf("Not copying dummy files\n");
        } else {
            for (int i = 0; i < 8; i++) {
                char new_filename[128];
                char dest_path[512];
                const char* source_path = dummies[i][0]; 
                //"at_%s.png" + "10001" -> "at_10001.png"
                snprintf(new_filename, sizeof(new_filename), dummies[i][1], final_level_id);
                // destination path "10001/at_10001.png"
                snprintf(dest_path, sizeof(dest_path), "%s/%s", final_level_id, new_filename);
                
                if (copy_file(source_path, dest_path)) {
                    printf("Copied %s\n", dest_path);
                } else {
                    printf("Could not find or copy %s\n", source_path);
                }
            }
        }
    }

    tinyfd_messageBox(
        "Finished",
        "Finished converting and copying files.\nPress OK to quit and open the final folder",
        "ok",
        "info",
        1
    );

    printf("\nAll processing is complete without crashing what a miracle :DDD\nOpening the level folder so we're not awkwardly sitting here...\n");

    char open_cmd[512];
    #ifdef _WIN32
        // Windows
        snprintf(open_cmd, sizeof(open_cmd), "explorer \"%s\"", final_level_id);
    #elif __APPLE__
        // macOS (please tell me this will never happen because what the actual fuck)
        snprintf(open_cmd, sizeof(open_cmd), "open \"%s\"", final_level_id);
    #else
        // Linux
        snprintf(open_cmd, sizeof(open_cmd), "xdg-open \"%s\"", final_level_id);
    #endif

    system(open_cmd);

    if (processing_mode == 2) {
        printf("Saving settings for quick run...\n");

        FILE *save_file = fopen("save.txt", "w");
        if (save_file != NULL) {
            fprintf(save_file, "%s\n%s\n", ssf_path, final_level_id);
            fclose(save_file);
        } else {
            printf("ERROR: Could not save settings for some reason (storage full?)\n");
            tinyfd_messageBox("Error", "Could not save settings (everything else is fine for some reason but this shouldn't be happening)", "ok", "error", 0);
            return 1;
        }
    }

    printf("Finished! Now let me rest in peace -w-");
    return 0;
}