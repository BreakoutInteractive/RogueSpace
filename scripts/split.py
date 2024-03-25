from PIL import Image
import os
import sys

def extract_subregions(input_image_path, output_folder):
    # Create output folder if it doesn't exist
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    # Open the input image
    image = Image.open(input_image_path)

    # Get the dimensions of the input image
    width, height = image.size

    # Define the size of subregions
    subregion_size_height = 128
    subregion_size_width = 128
    image_name = os.path.splitext(os.path.basename(input_image_path))[0]

    # Iterate over the image and extract subregions
    for y in range(0, height, subregion_size_height):
        for x in range(0, width, subregion_size_width):
            # Define the region to extract
            box = (x, y, x + subregion_size_width, y + subregion_size_height)

            # Crop the subregion
            subregion = image.crop(box)

            # Save the subregion as PNG
            output_path = os.path.join(output_folder, f"{image_name}_{x}_{y}.png")
            subregion.save(output_path, "PNG")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 split.py input_image_path output_folder")
        sys.exit(1)

    input_image_path = sys.argv[1]
    output_folder = sys.argv[2]

    extract_subregions(input_image_path, output_folder)
