# Usagi Engine, Copyright © Vitei, Inc. 2013
#packable_struct.rb
#Adds PackableStruct, a helper which adds a "pack" method to Structs

PACK_CODES = { cstring:   "a",
               u8:        "C",
               u16:       "S",
               u32:       "L",
               u64:       "Q",
               s8:        "c",
               s16:       "s",
               s32:       "l",
               s64:       "q",
               u16_le:    "S<",
               u32_le:    "L<",
               u64_le:    "Q<",
               s16_le:    "s<",
               s32_le:    "l<",
               s64_le:    "q<",
               u16_be:    "S>",
               u32_be:    "L>",
               u64_be:    "Q>",
               s16_be:    "s>",
               s32_be:    "l>",
               s64_be:    "q>",
               float:     "f",
               double:    "d",
               float_le:  "e",
               double_le: "E",
               float_be:  "g",
               double_be: "G",
               utf8:      "U",
               nullbyte:  "x" }

def to_packcode(v)
  if v.kind_of? Hash
    v.map{ |t, n| "#{to_packcode t}#{n.to_s}" }.join("")
  elsif v.respond_to? "map"
    v.map{ |t| to_packcode t }.join("")
  else
    PACK_CODES[v]
  end
end

def PackableStruct(values={})
  packcode = values.map{|k, v| to_packcode v}.join("")
  Struct.new(*values.map{|k, v| k}) do
    define_method(:pack) do
      to_a.pack packcode
    end
  end
end

