class File
  def self.exists?(path)
    exist?(path)
  end
end

module ProtocolBuffers
  class Field
    attr_reader :name, :otype

    def initialize(name, default_value = nil, otype: :required, repeated: false)
      @name = name.to_sym
      @default_value = default_value
      @otype = otype
      @repeated = repeated
    end

    def default_value
      @default_value
    end

    def repeated?
      @repeated
    end
  end

  class Field::MessageField < Field
    attr_reader :proxy_class

    def initialize(name, proxy_class, otype: :required, repeated: false)
      super(name, repeated ? [] : {}, otype: otype, repeated: repeated)
      @proxy_class = proxy_class
    end
  end

  class Field::StringField < Field
  end
end

module EntityHierarchyValidationPb
  class Message
    class << self
      def fields
        @fields ||= {}
      end

      def field(name, default_value = nil, otype: :required, repeated: false)
        field_class = default_value.is_a?(String) || default_value.nil? ? ProtocolBuffers::Field::StringField : ProtocolBuffers::Field
        fields[name.to_sym] = field_class.new(name, default_value, otype: otype, repeated: repeated)
      end

      def message_field(name, proxy_class, otype: :required, repeated: false)
        fields[name.to_sym] = ProtocolBuffers::Field::MessageField.new(name, proxy_class, otype: otype, repeated: repeated)
      end

      def fully_qualified_name
        name
      end
    end

    def initialize(values = nil, **kwargs)
      @values = values || {}
      @values = @values.merge(kwargs) if !kwargs.empty?
    end

    def serialize(stream)
      stream << serialize_to_string
    end

    def serialize_to_string
      "#{self.class.name}{#{serialize_hash(@values)}}"
    end

    private

    def serialize_hash(hash)
      hash.keys.sort_by(&:to_s).map do |key|
        "#{key}=#{serialize_value(hash[key])}"
      end.join(",")
    end

    def serialize_value(value)
      case value
      when EntityHierarchyValidationPb::Message
        value.serialize_to_string
      when Hash
        "{#{serialize_hash(value)}}"
      when Array
        "[#{value.map { |item| serialize_value(item) }.join(",")}]"
      when String
        value.inspect
      else
        value.to_s
      end
    end
  end
end

module Usg
  class ComponentHeader < EntityHierarchyValidationPb::Message
    field :id, 0
    field :byteLength, 0
  end

  class HierarchyHeader < EntityHierarchyValidationPb::Message
    field :entityCount, 0
  end

  class EntityHeader < EntityHierarchyValidationPb::Message
    field :componentCount, 0
    field :childEntityCount, 0
    field :initializerEventCount, 0
  end

  class InitializerEventHeader < EntityHierarchyValidationPb::Message
    field :id, 0
    field :byteLength, 0
    field :check, 0
  end

  module Ai
    module Pb
    end

    module Components
    end
  end

  module Components
    class Vector3f < EntityHierarchyValidationPb::Message
      field :x, 0.0
      field :y, 0.0
      field :z, 0.0
    end

    class Quaternionf < EntityHierarchyValidationPb::Message
      field :x, 0.0
      field :y, 0.0
      field :z, 0.0
      field :w, 1.0
    end

    class Identifier < EntityHierarchyValidationPb::Message
      field :name, ""
    end

    class TransformComponent < EntityHierarchyValidationPb::Message
      message_field :position, Vector3f
      message_field :rotation, Quaternionf
      field :bInheritFromParent, true
    end

    class HealthComponent < EntityHierarchyValidationPb::Message
      field :fLife, 1.0
      field :uKillerTeam, 0
      field :iKillerNUID, 0
    end
  end

  module Events
    class IncreaseHealthEvent < EntityHierarchyValidationPb::Message
      field :amount, 0.0
    end
  end

  module Exchange
    module BoneBillboardMode
      OFF = 0
    end
  end

  module LightKind
    DIRECTIONAL = 0
  end
end

module Events
end

module Processor
  class Merge < EntityHierarchyValidationPb::Message
    field :entityWithID, nil, otype: :optional
  end
end
