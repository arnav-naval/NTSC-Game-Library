import argparse
from pathlib import Path
from PIL import Image
import numpy as np

# Standard 16-color palette using HTML/CSS named color values
RGB_TO_INDEX = {
    (0, 0, 0): 0,        # black
    (199, 40, 37): 1,      # navy
    (72, 87, 39): 2,      # green
    (218, 45, 1): 3,    # teal
    (46, 113, 74): 4,      # maroon
    (82, 86, 108): 5,    # purple
    (46, 177, 40): 6,    # olive
    (158, 187, 44): 7,  # silver
    (24, 57, 231): 8,  # gray
    (194, 75, 248): 9,      # blue
    (69, 67, 97): 10,     # lime
    (247, 74, 127): 11,   # aqua
    (4, 76, 239): 12,     # red
    (81, 92, 259): 13,   # fuchsia
    (4, 158, 155): 14,   # yellow
    (255, 255, 255): 15, # white
}

# Create reverse mapping (index to RGB)
INDEX_TO_RGB = {index: rgb for rgb, index in RGB_TO_INDEX.items()}

def get_4bit_color_index(r, g, b):
    color = (r, g, b)
    if color not in RGB_TO_INDEX:
        raise ValueError(f"Color RGB={color} is not in the 16-color palette")
    return RGB_TO_INDEX[color]

def get_rgb_from_index(index):
    if index not in INDEX_TO_RGB:
        raise ValueError(f"Index {index} is not in the 16-color palette")
    return INDEX_TO_RGB[index]

def display_pixel_values(image_path):
    """
    Load a PNG image and display its pixel values
    
    Args:
        image_path (str): Path to the PNG image file
    """
    try:
        # Open the image
        img = Image.open(image_path)
        
        # Convert image to RGB mode if it isn't already
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        # Convert image to numpy array for easier handling
        pixel_array = np.array(img)
        
        # Get image dimensions
        height, width = pixel_array.shape[:2]
        
        print(f"Image size: {width}x{height}")
        print("\nPixel values (RGB):")
        
        # Display pixel values
        check = 0
        for y in range(height):
            if check > 10: break
            check += 1
            for x in range(width):
                r, g, b = pixel_array[y, x]
                try:
                    four_bit_index = get_4bit_color_index(r, g, b)
                except ValueError as e:
                    print(f"Pixel ({x}, {y}): {str(e)}")
                
    except Exception as e:
        print(f"Error loading image: {str(e)}")

def pack_image_to_binary(image_path, output_path):
    """
    Pack a PNG image into a binary file where each byte contains two 4-bit color indices
    
    Args:
        image_path (str): Path to the PNG image file
        output_path (str): Path where the binary file will be saved
    """
    try:
        # Open and prepare the image
        img = Image.open(image_path)
        if img.mode != 'RGB':
            img = img.convert('RGB')
        pixel_array = np.array(img)
        height, width = pixel_array.shape[:2]
        
        # Create binary data array
        binary_data = bytearray()
        
        # Process two pixels at a time
        for y in range(height):
            for x in range(0, width, 2):
                # Get first pixel's color index
                r1, g1, b1 = pixel_array[y, x]
                print(x, y)
                index1 = get_4bit_color_index(r1, g1, b1)
                
                # Get second pixel's color index (or use 0 if at the edge)
                if x + 1 < width:
                    r2, g2, b2 = pixel_array[y, x + 1]
                    print(x + 1, y)
                    index2 = get_4bit_color_index(r2, g2, b2)
                else:
                    index2 = 0
                
                # Pack two 4-bit indices into one byte
                byte = (index1 << 4) | index2
                binary_data.append(byte)
        
        # Write to binary file
        with open(output_path, 'wb') as f:
            f.write(binary_data)
            
        print(f"Successfully packed image to {output_path}")
        print(f"Image size: {width}x{height}")
        print(f"Binary file size: {len(binary_data)} bytes")
            
    except Exception as e:
        print(f"Error packing image: {str(e)}")

def read_binary_file(binary_path, width, height):
    """
    Read a packed binary file and display the color information for each pixel
    
    Args:
        binary_path (str): Path to the binary file
        width (int): Original image width
        height (int): Original image height
    """
    try:
        with open(binary_path, 'rb') as f:
            binary_data = f.read()
            
        print(f"Binary file size: {len(binary_data)} bytes")
        print(f"Expected size for {width}x{height}: {((width + 1) // 2) * height} bytes")
        
        pixel_count = 0
        for byte_index, byte in enumerate(binary_data):
            # Extract two 4-bit indices from each byte
            index1 = (byte >> 4) & 0x0F  
            index2 = byte & 0x0F         
            
            # Calculate pixel positions
            x1 = (byte_index * 2) % width
            y1 = (byte_index * 2) // width
            
            # Display first pixel
            if pixel_count < width * height:
                rgb = get_rgb_from_index(index1)
                pixel_count += 1
            
            # Display second pixel
            x2 = x1 + 1
            if x2 < width and pixel_count < width * height:
                rgb = get_rgb_from_index(index2)
                pixel_count += 1
                
    except Exception as e:
        print(f"Error reading binary file: {str(e)}")


#IMPORTANT FUNCTION FOR GENERATING SPRITE FILES!!!!!
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
def generate_sprite_files(image_path, output_dir):
    """
    Generate both .h and .c files for sprite data with 16-bit values
    
    Args:
        image_path (str): Path to the PNG image file
        output_dir (str): Directory where the output files will be saved
    """
    try:
        # Open and prepare the image
        img = Image.open(image_path)

        # Then convert to RGB, this will handle the transparency properly
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        pixel_array = np.array(img)
        height, width = pixel_array.shape[:2]
        
        # Validate size
        if width % 4 or height % 4:
            raise ValueError("Image dimensions must be multiples of 4")
        
        #CHANGE SPRITE NAME HERE!!!!!!!!
        #!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
        #!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        sprite_name = "tilemap"
        header_path = Path(output_dir) / f"include/{sprite_name}.h"
        source_path = Path(output_dir) / f"src/{sprite_name}.c"
        
        # Generate header file content
        header_guard = f"{sprite_name.upper()}_SPRITE_H"
        header_content = [
            f"#ifndef {header_guard}",
            f"#define {header_guard}",
            "",
            '#include "common.h"',
            "",
            f"// Sprite dimensions for {sprite_name}",
            f"#define MAP_WIDTH_PIXELS {width}",
            f"#define PIXEL_WIDTH_BITS 4",
            f"#define MAP_WIDTH_BYTES (PIXEL_WIDTH_BITS * MAP_WIDTH_PIXELS / 8)",
            f"#define MAP_WIDTH (MAP_WIDTH_BYTES / 2)",
            f"#define MAP_HEIGHT {height}",
            "",
            "// Packed 4-bit color indices, four pixels per uint16_t",
            f"extern const uint16_t tilemap[MAP_HEIGHT][MAP_WIDTH];",
            "",
            f"#endif // {header_guard}",
            ""
        ]
        
        # Generate source file content
        source_content = [
            f'#include "{Path(header_path).name}"',
            "",
            "// Packed 4-bit color indices, four pixels per uint16_t",
            "const uint16_t tilemap[MAP_HEIGHT][MAP_WIDTH] = {",
        ]
        
        # Process pixels and create uint16_t array
        words_data = []
        for y in range(height):
            row_words = []
            for x in range(0, width, 4):  # Process 4 pixels at a time for uint16_t
                word = 0
                
                # Pack up to 4 pixels into each uint16_t
                for i in range(4):
                    if x + i < width:
                        r, g, b = pixel_array[y, x + i]
                        print(x + i, y)
                        index = get_4bit_color_index(r, g, b)
                    else:
                        index = 0
                    word |= (index << ((3 - i) * 4))
                
                row_words.append(f"0x{word:04X}")
            
            # Add row to data with proper formatting
            if (y != height - 1):
                words_data.append("    {" + ", ".join(row_words) + "},")
            else:
                words_data.append("    {" + ", ".join(row_words) + "}")
        
        # Add data and close source file
        source_content.extend(words_data)
        source_content.extend([
            "};",
            ""
        ])
        
        # Write header file
        with open(header_path, 'w') as f:
            f.write('\n'.join(header_content))
            
        # Write source file
        with open(source_path, 'w') as f:
            f.write('\n'.join(source_content))
            
        print(f"Successfully generated sprite files:")
        print(f"Header: {header_path}")
        print(f"Source: {source_path}")
        
    except Exception as e:
        print(f"Error generating sprite files: {str(e)}")

def main():

    parser = argparse.ArgumentParser(description='Convert PNG image to sprite data files')
    parser.add_argument('input', help='Input PNG image path')
    parser.add_argument('output_dir', help='Output directory for generated files')
    
    args = parser.parse_args()
    
    # Convert paths to Path objects
    input_path = Path(args.input)
    output_dir = Path(args.output_dir)
    
    # Validate input file exists
    if not input_path.exists():
        print(f"Error: Input file '{input_path}' does not exist")
        return
    
    # Create output directory if it doesn't exist
    output_dir.mkdir(parents=True, exist_ok=True)
    display_pixel_values(str(input_path))
    # Generate the files
    generate_sprite_files(str(input_path), str(output_dir))

if __name__ == "__main__":
    main()
