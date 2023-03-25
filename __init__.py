
import os
from dotenv import load_dotenv

# Load the .env file
load_dotenv(os.path.join(os.path.dirname(__file__), ".env"))

import configparser

# Load the config/config.cfg file
config = configparser.ConfigParser(strict=True)
config.read(os.path.join(os.path.dirname(__file__), "config/config.cfg"))