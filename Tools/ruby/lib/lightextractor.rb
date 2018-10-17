# Usagi Engine, Copyright © Vitei, Inc. 2013
require 'nokogiri'
require 'Engine/Framework/FrameworkComponents.pb.rb'
require 'Engine/Graphics/Lights/LightSpec.pb.rb'

require_relative 'matrix_util.rb'

module LightExtractor

  ##################
  # classes        #
  ##################
  class LightNode
    attr_accessor :name, :type, :position, :direction
    attr_accessor :has_shadow, :ambient, :diffuse
    attr_accessor :specular, :inner_angle, :outer_angle
    attr_accessor :atten_enabled, :atten_start, :atten_end

    def initialize(name)
      @name = name
      @type = LightKind::DIRECTIONAL
      @position = {}
      @direction = {}
      @has_shadow = false
      @ambient = {}
      @diffuse = {}
      @specular = {}
      @inner_angle = 0.0
      @outer_angle = 0.0
      @atten_enabled = false
      @atten_start = 0.0
      @atten_end = 10000.0
    end

    def to_object

      base = { 'kind' => @type, 'ambient' => @ambient,
        'diffuse' => @diffuse, 'specular' => @specular, 'bShadow' => @has_shadow }
      atten = { 'bEnabled' => @atten_enabled, 'fNear' => @atten_start, 'fEnd' => atten_end }
      spot = { 'fInnerAngle' => @inner_angle, 'fOuterAngle' => @outer_angle }
      spec = {'direction' => @direction, 'base' => base, 'atten' => atten, 'spot' => spot}
      # Projection lights not available in models
      object = {'Identifier'=>{'name' => @name},
        'LightComponent' => {'spec' => spec} 
      }

      if @children.length > 0
        object['Children'] = []

        @children.each do |c|
          object['Children'] << c.to_object
        end
      end

      return object
    end
  end

  ##################
  # constants      #
  ##################
  TRANSFORM_FIELDS = ['Scale', 'Rotate', 'Translate']
  COMPONENTS = ['X', 'Y', 'Z']

  def self.extract(model_filename, rootBone)
    doc = Nokogiri::XML(File.read(model_filename))

    queries = {:lighting => 'lighting', :lights => 'light_array > light', :name => 'name',
      :parent => 'parent_name', :type => 'type' }


    lighting = doc.at_css(queries[:lighting])


    return rootBone.to_object
  end
end
