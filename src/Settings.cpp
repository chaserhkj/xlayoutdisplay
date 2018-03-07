#include "Settings.h"
#include "Monitors.h"
#include "util.h"

#include <cstring>
#include <getopt.h>

using namespace std;

#define USAGE "Usage: %s [-h] [-i] [-n] [-o order] [-p primary] [-q]\n"

#define SETTINGS_FILE_COMMENT_CHAR '#'
#define SETTINGS_FILE_SEPS " =\n,"
#define SETTINGS_FILE_LINE_MAX 512
#define SETTINGS_FILE_KEY_MIRROR "mirror"
#define SETTINGS_FILE_KEY_ORDER "order"
#define SETTINGS_FILE_KEY_PRIMARY "primary"
#define SETTINGS_FILE_KEY_QUIET "quiet"

Settings::Settings(int argc, char **argv) {

    // load persistent settings from ~/.xLayoutDisplay
    loadUserSettings(resolveTildePath(".xLayoutDisplay"));

    // override with CLI settings
    loadCliSettings(argc, argv);
}

// print help and exit with success
void printHelp(char *progPath) {
    char *progName = basename(progPath);
    printf(""
                   "Detects and arranges outputs in a left to right manner.\n"
                   "Invokes xrandr to perform arrangement.\n"
                   "Highest resolution and refresh are used for each output.\n"
                   "Outputs starting with \"%s\" are disabled if the laptop lid is closed as per /proc/acpi/button/lid/.*/state\n"
                   "Outputs are ordered via Xrandr default.\n"
                   "The first output will be primary unless -p specified.\n"
                   "\n", LAPTOP_OUTPUT_PREFIX
    );
    printf(USAGE, progName);
    printf(""
                   "  -h  print this help text and exit\n"
                   "  -i  print information about current outputs and exit\n"
                   "  -m  mirror outputs using the lowest common resolution\n"
                   "  -n  perform a trial run and exit\n"
                   "  -o  order of outputs, space/comma delimited\n"
                   "  -p  primary output\n"
                   "  -q  suppress feedback\n"
    );
    printf("\n"
                   "e.g.: %s -o DP-0,HDMI-0 -p HDMI-0\n"
                   "  arranges DP-0 left, HDMI-0 right, with any remaining outputs further right\n"
                   "  HDMI-0 set as primary\n"
                   "", progName
    );
    exit(EXIT_SUCCESS);
}

// print usage and help hint then exit with failure
void usage(char *progPath) {
    char *progName = basename(progPath);
    fprintf(stderr, USAGE, progName);
    fprintf(stderr, "Try '%s -h' for more information.\n", progName);
    exit(EXIT_FAILURE);
}

// no, this does not get any tests
void Settings::loadCliSettings(int argc, char **argv) {

    // load command line settings
    int opt;
    while ((opt = getopt(argc, argv, "himno:p:q")) != -1) {
        switch (opt) {
            case 'h':
                help = true;
                break;
            case 'i':
                info = true;
                break;
            case 'm':
                mirror = true;
                break;
            case 'n':
                dryRun = true;
                break;
            case 'o':
                for (char *token = strtok(optarg, " ,"); token != nullptr; token = strtok(nullptr, " ,"))
                    order.emplace_back(token);
                break;
            case 'p':
                primary = optarg;
                break;
            case 'q':
                verbose = false;
                break;
            default:
                usage(argv[0]);
        }
    }
    if (argc > optind)
        usage(argv[0]);
    if (help)
        printHelp(argv[0]);
}

void Settings::loadUserSettings(const string settingsFilePath) {
    char line[SETTINGS_FILE_LINE_MAX];
    char *key, *val;

    // read settings file, if it exists
    FILE *settingsFile = fopen(settingsFilePath.c_str(), "r");
    if (settingsFile) {

        // read each line
        while (fgets(line, SETTINGS_FILE_LINE_MAX, settingsFile)) {

            // key
            key = strtok(line, SETTINGS_FILE_SEPS);

            // skip comments
            if (key != nullptr && key[0] != SETTINGS_FILE_COMMENT_CHAR) {

                // value
                val = strtok(nullptr, SETTINGS_FILE_SEPS);
                if (val == nullptr)
                    throw invalid_argument(
                            string() + "missing value for key '" + key + "' in '" + settingsFilePath + "'");

                if (strcasecmp(key, SETTINGS_FILE_KEY_MIRROR) == 0) {
                    mirror = strcasecmp(val, "true") == 0;
                } else if (strcasecmp(key, SETTINGS_FILE_KEY_ORDER) == 0) {
                    while (val) {
                        order.emplace_back(val);
                        val = strtok(nullptr, SETTINGS_FILE_SEPS);
                    }
                } else if (strcasecmp(key, SETTINGS_FILE_KEY_PRIMARY) == 0) {
                    primary = val;
                } else if (strcasecmp(key, SETTINGS_FILE_KEY_QUIET) == 0) {
                    verbose = strcasecmp(val, "true") != 0;
                } else {
                    throw invalid_argument(string() + "invalid key '" + key + "' in '" + settingsFilePath + "'");
                }
            }
        }
        fclose(settingsFile);
    }
}
