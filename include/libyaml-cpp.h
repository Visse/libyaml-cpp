#pragma once


#ifndef LIBYAML_CPP_API
#   ifdef LIBYAML_CPP_STATIC
#       define LIBYAML_CPP_API
#   elif LIBYAML_CPP_EXPORT
#       define LIBYAML_CPP_API __declspec(dllexport)
#   else
#       define LIBYAML_CPP_API __declspec(dllimport)
#   endif
#endif


typedef struct yaml_document_s yaml_document_t;
typedef struct yaml_node_s yaml_node_t;

// for std::shared_ptr
#include <memory>
// for std::iterator
#include <iterator>
// for std::pair
#include <utility>
// for std::istream
#include <iosfwd>

namespace libyaml
{
    struct Mark {
        int byte_offset   = 0,
            col           = 0,
            line          = 0;
    };

    struct NodeIterator;

    class Node {
        std::shared_ptr<yaml_document_t> mDocument;
        yaml_node_t *mNode = nullptr;
    public:
        enum class Type {
            Null,
            Scalar,
            Sequence,
            Map
        };

        using iterator = NodeIterator;

        static LIBYAML_CPP_API Node LoadString( const char *str );
        static LIBYAML_CPP_API Node LoadString( const char *str, size_t len );
        static LIBYAML_CPP_API Node LoadStream( std::istream &stream );

    public:
        Node() = default;
        Node( const Node& ) = default;
        Node( Node&& ) = default;
        ~Node() = default;

        Node& operator = ( const Node& ) = default;
        Node& operator = ( Node&& ) = default;

        Node( std::shared_ptr<yaml_document_t> document, yaml_node_t *node ) :
            mDocument(document),
            mNode(node)
        {}

        LIBYAML_CPP_API Type type() const;

        LIBYAML_CPP_API Mark startMark() const;
        LIBYAML_CPP_API Mark endMark() const;

        LIBYAML_CPP_API const char* scalar() const;

        LIBYAML_CPP_API Node operator [] ( int i ) const;
        LIBYAML_CPP_API Node operator [] ( const char *str ) const;

        LIBYAML_CPP_API size_t size() const;

        iterator begin() const;
        iterator end() const;

        explicit operator bool () const {
            return type() != Type::Null;
        }
        bool operator ! () const {
            return type() == Type::Null;
        }
        friend bool operator == ( const Node &lhs, const Node &rhs ) {
            return lhs.mNode == rhs.mNode;
        }
        friend bool operator != ( const Node &lhs, const Node &rhs ) {
            return lhs.mNode != rhs.mNode;
        }

        bool isScalar() const {
            return type() == Type::Scalar;
        }
        bool isSequence() const {
            return type() == Type::Sequence;
        }
        bool isMap() const {
            return type() == Type::Map;
        }
    };

    struct IteratorElement :
        public Node,
        public std::pair<Node, Node>
    {
        IteratorElement() = default;
        IteratorElement( const Node &node ) :
            Node(node)
        {}
        IteratorElement( const std::pair<Node, Node> &pair ) :
            std::pair<Node,Node>(pair)
        {};
    };

    class NodeIterator :
        public std::iterator<std::random_access_iterator_tag, IteratorElement>
    {
        std::shared_ptr<yaml_document_t> mDocument;
        yaml_node_t *mNode;
        intptr_t mIdx = 0;

    public:
        NodeIterator() = default;
        NodeIterator( const NodeIterator& ) = default;
        NodeIterator( NodeIterator&& ) = default;
        ~NodeIterator() = default;

        NodeIterator& operator = ( const NodeIterator& ) = default;
        NodeIterator& operator = ( NodeIterator&& ) = default;

        NodeIterator( std::shared_ptr<yaml_document_t> document, yaml_node_t *node, intptr_t idx ) :
            mDocument(document),
            mNode(node),
            mIdx(idx)
        {}

        NodeIterator& operator ++ () {
            advance(1);
            return *this;
        }
        NodeIterator operator ++ (int) {
            NodeIterator copy(*this);
            advance(1);
            return copy;
        }
        NodeIterator& operator -- () {
            advance(-1);
            return *this;
        }
        NodeIterator operator -- (int) {
            NodeIterator copy(*this);
            advance(-1);
            return copy;
        }
        
        NodeIterator& operator += ( ptrdiff_t n ) {
            advance(n);
            return *this;
        }
        NodeIterator& operator -= ( ptrdiff_t n ) {
            advance(-n);
            return *this;
        }

        IteratorElement operator * () {
            return current();
        }

        // @todo figure out 'operator ->'

        friend NodeIterator operator + ( const NodeIterator &iter, ptrdiff_t n ) {
            NodeIterator copy(iter);
            copy.advance(n);
            return copy;
        }
        friend NodeIterator operator + ( ptrdiff_t n, const NodeIterator &iter ) {
            NodeIterator copy(iter);
            copy.advance(n);
            return copy;
        }
        
        friend NodeIterator operator - ( const NodeIterator &iter, ptrdiff_t n ) {
            NodeIterator copy(iter);
            copy.advance(-n);
            return copy;
        }
        friend NodeIterator operator - ( ptrdiff_t n, const NodeIterator &iter ) {
            NodeIterator copy(iter);
            copy.advance(-n);
            return copy;
        }

        friend ptrdiff_t operator - ( const NodeIterator &lhs, const NodeIterator &rhs ) {
            return lhs.mIdx - rhs.mIdx;
        }

        friend bool operator == ( const NodeIterator &lhs, const NodeIterator &rhs ) {
            // no need to compare documents - if the node is equal so must the documents
            if (lhs.mNode != rhs.mNode) return false;
            if (lhs.mIdx != rhs.mIdx) return false;
            return true;
        }
        friend bool operator != ( const NodeIterator &lhs, const NodeIterator &rhs ) {
            return !operator == (lhs, rhs);
        }

    private:
        void advance( ptrdiff_t n ) {
            mIdx += n;
        }
        LIBYAML_CPP_API IteratorElement current() const;
    };

    
    inline NodeIterator Node::begin() const {
        return NodeIterator(mDocument, mNode, 0);
    }
    inline NodeIterator Node::end() const {
        return NodeIterator(mDocument, mNode, size());
    }
}



