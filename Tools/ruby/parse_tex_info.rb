# Usagi Engine, Copyright © Vitei, Inc. 2013

def indent(line)
  line ? line.sub(/[^ ].*/, "").length : -1
end

def parse_props(i, props)
  out = {}

  while props && indent(props.first) >= i do
    key, value = props.shift.split(":", 2).map{ |x| x.strip }
    value = parse_props(indent(props.first), props) if value.empty?

    out.merge!({key => value})
  end

  out
end

def parse(info)
  out = []

  lines = info.lines.to_a
  while lines && !lines.empty?
    lines.shift
    img_props = parse_props(indent(lines.first), lines)
    out << img_props
  end

  out
end

def get_mipmaps(filename)
  File.open(filename, "rb") do |f|
    hdr = f.read[4..31].unpack('L*')
    hdr[6].to_i
  end
end

def get_textype(images)
  case images.length
  when 1
    "2D"
  when 6
    "cube"
  else
    raise "Unsupported dds: #{images.length.to_s} embedded images"
  end
end

def get_cubemap_dir(idx)
  ["+x", "-x", "+y", "-y", "+z", "-z"][idx]
end

