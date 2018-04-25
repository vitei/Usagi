# Usagi Engine, Copyright © Vitei, Inc. 2013
require 'nokogiri'

# Define some helper functions to convert attribute values of Nokogiri
# XML nodes directly into primitive types.
class Nokogiri::XML::Node
  def attribute_content(name)
    attribute(name).content
  end

  def attribute_to_i(name)
    attribute_content(name).to_i
  end

  def attribute_to_f(name)
    attribute_content(name).to_f
  end

  def attribute_to_s(name)
    attribute_content(name)
  end

  def attribute_to_b(name, defaultValue)
    value = defaultValue

    if (key?(name))
      stringValue = attribute_content(name)

      value = (stringValue == "true") ? true : false
    end

    return value
  end
end
