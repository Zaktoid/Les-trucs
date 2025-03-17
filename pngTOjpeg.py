from PIL import Image
import sys

def convert_png_to_jpeg(png_path, jpeg_path=None):
    """
    Convert a PNG image to JPEG format
    
    Args:
        png_path (str): Path to the source PNG file
        jpeg_path (str): Optional path for the output JPEG file. If not provided,
                        creates a file with the same name but .jpg extension
    """
    try:
        # Open the PNG image
        with Image.open(png_path) as img:
            # Convert to RGB mode if necessary (in case of RGBA images)
            if img.mode in ('RGBA', 'LA'):
                bg = Image.new('RGB', img.size, (255, 255, 255))
                bg.paste(img, mask=img.split()[-1])
                img = bg
            
            # Generate output path if not provided
            if not jpeg_path:
                jpeg_path = png_path.rsplit('.', 1)[0] + '.jpg'
            
            # Save as JPEG
            img.save(jpeg_path, 'JPEG', quality=95)
            return True
    except Exception as e:
        print(f"Error converting image: {e}")
        return False

# Example usage
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py input.png [output.jpg]")
    else:
        png_file = sys.argv[1]
        jpeg_file = sys.argv[2] if len(sys.argv) > 2 else None
        if convert_png_to_jpeg(png_file, jpeg_file):
            print("Conversion successful")
