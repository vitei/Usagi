# Usagi Engine, Copyright © Vitei, Inc. 2013
require 'nokogiri'
require 'Engine/Framework/FrameworkComponents.pb.rb'
require 'Engine/Scene/Model/Skeleton.pb.rb'

require_relative 'matrix_util.rb'

module SkeletonExtractor
  module BoneType
    NORMAL = 0
    INTERMEDIATE = 1
    BILLBOARD = 2
  end


  ##################
  # classes        #
  ##################
  class BoneNode
    attr_accessor :name, :children, :scale, :rotate, :translate
    attr_accessor :is_root, :type, :billboard_mode

    def initialize(name, is_root=false)
      @name = name
      @children = []
      @light_children = []
      @camera_children = []
      @scale = {}
      @rotate = {}
      @translate = {}
      @is_root = is_root
      @type = BoneType::NORMAL
      @billboard_mode = Usg::Exchange::BoneBillboardMode::OFF
    end

    def add_light(light)
      @light_children << light
    end

    def add_camera(camera)
      @camera_children << camera
    end

    def find(name, depth)
      if name == @name
        return [self, depth]
      end

      @children.each do |c|
        didMatch = c.find(name, depth+1)

        if didMatch[1] != -1
          return didMatch
        end
      end

      return [nil, -1]
    end

    def to_object
      rotationMatrix = Matrix.rotation_matrix4x4(@rotate['x'], @rotate['y'], @rotate['z'])
      rotationMatrix.set_translation(@translate['x'], @translate['y'], @translate['z'])

      object = {'Identifier'=>{'name' => @name},
        'TransformComponent' => {'position' => Hash[@translate],
          'rotation' => rotationMatrix.rotation_matrix_to_quaternion()},
        'BoneComponent' => {'m_scale' => @scale,
          'm_rotate' => @rotate, 'm_translate' => @translate},}

      case @type
      when BoneType::NORMAL
      when BoneType::INTERMEDIATE
        object['IntermediateBone'] = {}
        object.delete('BoneComponent')
        object.delete('Identifier')
      when BoneType::BILLBOARD
        object.delete('TransformComponent')
        object['Billboard'] = {'mode' => @billboard_mode}
      end

      if @is_root
        object['StateComponent'] = {'current' => Usg::STATUS::ACTIVE} 
      end

      if @children.length > 0 or @light_children.length > 0 or @camera_children.length > 0
        object['Children'] = []
      end

      if @children.length > 0 
        @children.each do |c|
          object['Children'] << c.to_object
        end
      end

      if @light_children.length > 0 
        @light_children.each do |c|
          object['Children'] << c.to_object
        end
      end

      if @camera_children.length > 0 
        @camera_children.each do |c|
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

  def self.extract(model_filename)
    doc = Nokogiri::XML(File.read(model_filename))

    queries = {:skeleton => 'hierarchy', :bones => 'bone_array > bone', :name => 'name',
      :parent => 'parent_name', :billboard => 'billboard',
    :billboard_disabled => 'none'}


    skeleton = doc.at_css(queries[:skeleton])

    if skeleton.nil?
      # A hack to match the forced fake root bone for all skeletons
      rootBone = BoneNode.new("root_bone", true)

      rootBone.translate['x'] = 0.0
      rootBone.translate['y'] = 0.0
      rootBone.translate['z'] = 0.0

      rootBone.rotate['x'] = 0.0
      rootBone.rotate['y'] = 0.0
      rootBone.rotate['z'] = 0.0

      rootBone.scale['x'] = 1.0
      rootBone.scale['y'] = 1.0
      rootBone.scale['z'] = 1.0

      return rootBone
    end

    rootName = ''

    rootBone = skeleton.at_css('bone_array > bone[index="0"]')
    rootName = rootBone['name']

    rootBone = BoneNode.new(rootName, true)

    skeleton.css(queries[:bones]).sort.each do |b|
      boneName = b[queries[:name]]
      parentBoneName = b[queries[:parent]]

      next if /_LOD[^0]$/.match(boneName) != nil

      node = rootBone

      if parentBoneName.length > 1
        node = BoneNode.new(boneName)
        parentNode = rootBone.find(parentBoneName, 1)

        if parentNode[0] == nil
          raise "couldn't find parent bone '#{parentBoneName}' in input file '#{model_filename}'"
        end

        parentNode[0].children << node
      end


      SkeletonExtractor::TRANSFORM_FIELDS.map{|f| f.downcase}.each do |field_name|
        values = b[field_name].split(' ').slice(0, 3)

        SkeletonExtractor::COMPONENTS.each_with_index do |c, index|
          node.instance_eval("#{field_name}['#{c.downcase}'] = #{values[index].to_f}")
        end
      end

      billboard_mode = b[queries[:billboard]]

      if billboard_mode != queries[:billboard_disabled]
        node.type = BoneType::INTERMEDIATE
        nodeCopy = Marshal.load(Marshal.dump(node))
        nodeCopy.type = BoneType::BILLBOARD
        nodeCopy.billboard_mode = Usg::Exchange::BoneBillboardMode::const_get(billboard_mode.upcase)
        node.children << nodeCopy
      end
    end

    return rootBone
  end
end
