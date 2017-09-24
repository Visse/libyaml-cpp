#include "libyaml-cpp.h"

#include "yaml.h"

#include <cassert>

namespace libyaml
{
    namespace
    {
        bool utf8_equal( const char *str1, size_t len1,
                         const char *str2, size_t len2 )
        {
            // @todo proper utf8 comparason
            if (len1 != len2) return false;
            return memcmp(str1, str2, len1) == 0;
        }

        std::shared_ptr<yaml_document_t> CreateDocument()
        {
            return std::shared_ptr<yaml_document_t>(new yaml_document_t,
                [](yaml_document_t *doc) {
                    yaml_document_delete(doc);
                    delete doc;
                }
            );
        }

        struct ParserDelete {
            void operator () ( yaml_parser_t *parser ) const {
                yaml_parser_delete(parser);
            }

        };
    }


    LIBYAML_CPP_API Node Node::LoadString( const char *str )
    {
        size_t len = strlen(str);
        return LoadString(str, len);
    }

    LIBYAML_CPP_API Node Node::LoadString( const char *str, size_t len )
    {
        yaml_parser_t parser;
        std::unique_ptr<yaml_parser_t, ParserDelete> parserDelete(&parser);

        if (yaml_parser_initialize(&parser) != 1) {
            throw std::runtime_error("Failed to initilize yaml parser");
        }
        yaml_parser_set_encoding(&parser, YAML_UTF8_ENCODING);
        yaml_parser_set_input_string(&parser, (const unsigned char*)str, len);
        
        std::shared_ptr<yaml_document_t> document = CreateDocument();
        if (yaml_parser_load(&parser, document.get()) != 1) {
            // @todo include error information
            throw std::runtime_error("Failed to load yaml document!");
        }

        yaml_node_t *root = yaml_document_get_root_node(document.get());
        return Node(document, root);
    }

    LIBYAML_CPP_API Node Node::LoadStream( std::istream &stream )
    {
        yaml_parser_t parser;
        std::unique_ptr<yaml_parser_t, ParserDelete> parserDelete(&parser);

        if (yaml_parser_initialize(&parser) != 1) {
            throw std::runtime_error("Failed to initilize yaml parser");
        }

        yaml_read_handler_t *readHandler = [](void *data, unsigned char *buffer, size_t size, size_t *size_read) -> int {
            std::istream *stream = (std::istream*)data;
            stream->read((char*)buffer, size);
            *size_read = stream->gcount();
            return stream->fail() ? 1 : 0;
        };

        yaml_parser_set_encoding(&parser, YAML_UTF8_ENCODING);
        yaml_parser_set_input(&parser, readHandler, &stream);
        
        std::shared_ptr<yaml_document_t> document = CreateDocument();
        if (yaml_parser_load(&parser, document.get()) != 1) {
            // @todo include error information
            throw std::runtime_error("Failed to load yaml document!");
        }

        yaml_node_t *root = yaml_document_get_root_node(document.get());
        return Node(document, root);
    }

    LIBYAML_CPP_API Node::Type Node::type() const
    {
        if (mNode == nullptr) return Type::Null;

        switch (mNode->type) {
        case YAML_NO_NODE:
            return Type::Null;
        case YAML_SCALAR_NODE:
            return Type::Scalar;
        case YAML_SEQUENCE_NODE:
            return Type::Sequence;
        case YAML_MAPPING_NODE:
            return Type::Map;
        }
        return Type::Null;
    }

    LIBYAML_CPP_API Mark Node::startMark() const
    {
        if (!mNode) return Mark{};
        Mark mark;
            mark.byte_offset = mNode->start_mark.index;
            mark.line = mNode->start_mark.line;
            mark.col = mNode->start_mark.column;
        return mark;
    }

    LIBYAML_CPP_API Mark Node::endMark() const
    {
        if (!mNode) return Mark{};
        Mark mark;
            mark.byte_offset = mNode->end_mark.index;
            mark.line = mNode->end_mark.line;
            mark.col = mNode->end_mark.column;
        return mark;
    }

    LIBYAML_CPP_API const char* Node::scalar() const
    {
        if (type() != Type::Scalar) return nullptr;

        const auto &scalar = mNode->data.scalar;
        return (const char*)scalar.value;
    }

    LIBYAML_CPP_API Node Node::operator [] ( int i ) const
    {
        if (type() != Type::Sequence) return Node{};
        if (i < 0) return Node{};

        const auto &sequence = mNode->data.sequence;
        yaml_node_item_t *item = sequence.items.start + i;
        // is we out of range?
        if (sequence.items.top <= item) return Node{};

        yaml_node_t *node = yaml_document_get_node(mDocument.get(), *item);

        return Node{mDocument, node};
    }

    LIBYAML_CPP_API Node Node::operator [] ( const char *str ) const
    {
        if (type() != Type::Map) return Node{};

        size_t len = strlen(str);

        const auto &mapping = mNode->data.mapping;
        for (yaml_node_pair_t *iter = mapping.pairs.start; iter < mapping.pairs.top; ++iter) {
            yaml_node_t *key = yaml_document_get_node(mDocument.get(), iter->key);
            if (key == nullptr) continue;
            if (key->type != YAML_SCALAR_NODE) continue;

            const auto &scalar = key->data.scalar;
            if (!utf8_equal(str, len, (const char*) scalar.value, scalar.length))  continue;
            
            yaml_node_t *value = yaml_document_get_node(mDocument.get(), iter->value);
            return Node(mDocument, value);
        }

        return Node{};
    }

    LIBYAML_CPP_API size_t Node::size() const
    {
        switch (type()) {
        case Type::Null:
        case Type::Scalar:
            return 0;
        case Type::Sequence: {
            const auto &sequence = mNode->data.sequence;
            return sequence.items.top - sequence.items.start;
        } case Type::Map: {
            const auto &mapping = mNode->data.mapping;
            return mapping.pairs.top - mapping.pairs.start;
        }}
        return 0;
    }

    LIBYAML_CPP_API IteratorElement NodeIterator::current() const
    {
        if (!mNode) return IteratorElement{};
        if (mIdx < 0) return IteratorElement{};

        switch (mNode->type) {
        case YAML_NO_NODE:
        case YAML_SCALAR_NODE:
            return IteratorElement{};
        case YAML_SEQUENCE_NODE: {
            const auto &sequence = mNode->data.sequence;
            yaml_node_item_t *item = sequence.items.start + mIdx;
            if (sequence.items.top <= item) return IteratorElement{};

            yaml_node_t *node = yaml_document_get_node(mDocument.get(), *item);
            return Node(mDocument, node);
        } case YAML_MAPPING_NODE: {
            const auto &mapping = mNode->data.mapping;
            
            yaml_node_pair_t *item = mapping.pairs.start + mIdx;
            if (mapping.pairs.top <= item) return IteratorElement{};

            yaml_node_t *key = yaml_document_get_node(mDocument.get(), item->key);
            yaml_node_t *value = yaml_document_get_node(mDocument.get(), item->value);

            return std::make_pair(
                Node(mDocument, key),
                Node(mDocument, value)
            );
        }};
        return IteratorElement {};
    }
}