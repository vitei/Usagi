# Usagi Engine, Copyright © Vitei, Inc. 2013

module Scribe
  module Origin
    LOWER_LEFT = 0
    UPPER_LEFT = 1
  end

  # Convert R, G, and B values to a 24-bit integer pixel value
  def self.rgb_to_pixel(r, g, b)
    # pack 8-bit per channel values
    [b,g,r].pack("CCC")
  end

  def self.create_tga(path, spec={}, pixels)
    header_spec = {
      :width => nil,
      :height => nil,
      :origin => Origin::LOWER_LEFT,
      :pixel_depth => 24,
      :bytes_per_pixel => 3 # 1 byte each for RGB (no alpha)
    }.merge(spec)

    image_descriptor = 0

    if header_spec[:origin] == Origin::UPPER_LEFT
      image_descriptor |= 0x20
    elsif header_spec[:origin] != Origin::LOWER_LEFT
      raise "Origin must be upper left or lower left!"
    end

    header = [
              0, # ID length
              0, # color map
              2, # image type: uncompressed RGB
              0, # color map origin
              0, # color map length
              0, # color map depth
              0, # x origin
              0, # y origin
              header_spec[:width],
              header_spec[:height],
              header_spec[:pixel_depth],
              image_descriptor # image_descriptor
             ].pack("CCCvvCvvvvCC")

    output_file = File.open(path, "wb")
    output_file.write(header)

    pixels.each do |row|
      row.each {|pixel| output_file.write pixel}
    end

    output_file.close
  end
end
