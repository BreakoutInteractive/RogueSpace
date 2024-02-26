import numpy as np
from PIL import Image
import noise

def generate_space_image(width, height, num_galaxies, num_bright):
    # Create a blank image
    image = Image.new("RGB", (width, height), "black")
    pixels = image.load()

    # Generate galaxy positions and sizes
    galaxy_positions = np.random.randint(0, width, size=(num_galaxies, 2))
    galaxy_sizes = np.random.randint(1, 3, size=num_galaxies)

    # Add galaxies to the image
    for i in range(num_galaxies):
        x, y = galaxy_positions[i]
        size = galaxy_sizes[i]

        # Generate galaxy color based on random noise
        color = int(noise.pnoise2(x / 100, y / 100))
        color = max(0, min(255, color * 127 + 128))

        chance = np.random.rand()
        if (chance >= 0.75 and num_bright > 0):
            color = 255
            num_bright -= 1
            size *= 1.5
            size = int(size)

        # Draw a circular galaxy
        for dx in range(-size, size + 1):
            for dy in range(-size, size + 1):
                if 0 <= x + dx < width and 0 <= y + dy < height:
                    dist = np.sqrt(dx**2 + dy**2)
                    if dist <= size:
                        pixels[x + dx, y + dy] = (color, color, color)

    return image

def main():
    width, height = 1600, 900
    num_galaxies = 700
    num_bright = 50
    space_image = generate_space_image(width, height, num_galaxies, num_bright)
    space_image.save("space_background.png")
    space_image.show()

if __name__ == "__main__":
    main()
