import os

from hypeost import config

# Set the directories
GAME_DIR = os.getenv("GAME_DIR")
CSB_DIR = os.path.join(GAME_DIR, config.get(config.default_section, "csb_dir").strip('"'))
BNM_APM_DIR = os.path.join(GAME_DIR, config.get(config.default_section, "bnm_apm_dir").strip('"'))

# Define the CSB file prefixes
CSB_PREFIXES = {
    "M": "Music",
    "P": "Sound effects",
    "PA": "Animal sound effects",
    "PB": "Boss sound effects",
    "PE": "Enemy sound effects",
    "S": "Other sounds",
    "SP": "Ambient sounds",
}

def get_csb_files():
    files = []
    for file in os.listdir(CSB_DIR):
        if file.endswith(".csb"):
            files.append(os.path.join(CSB_DIR, file))
    return files

def get_bnm_apm_files():
    bnm_files = []
    apm_files = []

    for file in os.listdir(BNM_APM_DIR):
        if file.endswith(".bnm"):
            bnm_files.append(os.path.join(BNM_APM_DIR, file))
        elif file.endswith(".apm"):
            apm_files.append(os.path.join(BNM_APM_DIR, file))

    return bnm_files, apm_files

def main():
    print("CSB Files:")
    print(get_csb_files())
    print("\nBNM and APM Files:")
    print(get_bnm_apm_files())

if __name__ == "__main__":
    main()