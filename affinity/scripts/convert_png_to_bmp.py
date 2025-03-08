from PIL import Image
import os
import sys

def convert_png_to_bmp(png_filepath, bmp_filepath):
    """
    Converts a PNG image to BMP format.

    Args:
        png_filepath (str): The path to the input PNG file.
        bmp_filepath (str): The path to save the output BMP file.
    """
    try:
        img = Image.open(png_filepath)
        img.save(bmp_filepath, 'BMP')
        print(f"Successfully converted '{png_filepath}' to '{bmp_filepath}'")
    except FileNotFoundError:
        print(f"Error: PNG file '{png_filepath}' not found.")
    except Exception as e:
        print(f"An error occurred: {e}")

def convert_files_in_dir(directory):
    """
    Converts all PNG files in the given directory to BMP format.

    Args:
        directory (str): The path to the directory containing PNG files.
    """
    for filename in os.listdir(directory):
        if filename.endswith(".png"):
            png_filepath = os.path.join(directory, filename)
            bmp_filepath = os.path.join(directory, filename.replace(".png", ".bmp"))
            convert_png_to_bmp(png_filepath, bmp_filepath)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python convert_png_to_bmp.py <directory>")
        sys.exit(1)

    directory = sys.argv[1]
    if not os.path.isdir(directory):
        print(f"Error: '{directory}' is not a valid directory")
        sys.exit(1)

    convert_files_in_dir(directory)