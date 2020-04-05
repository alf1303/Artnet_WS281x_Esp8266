//Settings for manual strip control
#define int_mid 128;
#define int_full 255;
int colorSaturation = int_mid;
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor black(0, 0, 0);

//
void initTest()
{
  for (int i = 0 ; i < PixelCount ; i++)
    strip.SetPixelColor(i, red);
  strip.Show();
  delay(500);
  for (int i = 0 ; i < PixelCount ; i++)
    strip.SetPixelColor(i, green);
  strip.Show();
  delay(500);
  for (int i = 0 ; i < PixelCount ; i++)
    strip.SetPixelColor(i, blue);
  strip.Show();
  delay(500);
  for (int i = 0 ; i < PixelCount ; i++)
    strip.SetPixelColor(i, black);
  strip.Show();
}