#include "libyaml-cpp.h"


#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>

using namespace libyaml;

int main()
{
    Node root = Node::LoadString(
R"(
scalar: hello
sequence:
    - hello
    - world
map:
    hello: world
    foo: bar
)"
    );


    assert (root.isMap());

    Node scalarNode = root["scalar"];
    assert (scalarNode.isScalar());
    assert (std::string("hello") == scalarNode.scalar());

    Node sequenceNode = root["sequence"];
    assert (sequenceNode.isSequence());
    assert (sequenceNode.size() == 2);
    
    Node seq0 = sequenceNode[0];
    assert (seq0.isScalar());
    Node seq1 = sequenceNode[1];
    assert (seq1.isScalar());
    Node seq2 = sequenceNode[2];
    assert (!seq2);

    assert (seq0.scalar() == std::string("hello"));
    assert (seq1.scalar() == std::string("world"));
    assert (seq2.scalar() == nullptr);

    Node mapNode = root["map"];
    assert (mapNode.isMap());
    assert (mapNode.size() == 2);
    
    Node map0 = mapNode["hello"];
    assert (map0.isScalar());
    Node map1 = mapNode["foo"];
    assert (map1.isScalar());
    Node map2 = mapNode["abc"];
    assert (!map2);
    
    assert (map0.scalar() == std::string("world"));
    assert (map1.scalar() == std::string("bar"));
    assert (map2.scalar() == nullptr);

    auto iter = sequenceNode.begin();
    auto end = sequenceNode.end();

    assert (iter != end);
    assert (Node(*iter) == seq0);
    ++iter;
    assert (iter != end);
    assert (Node(*iter) == seq1);
    ++iter;
    assert (iter == end);

    iter = mapNode.begin();
    end = mapNode.end();
        
    assert (iter != end);
    assert ((*iter).second == map0);
    assert ((*iter).first.scalar() == std::string("hello"));
    ++iter;
    assert (iter != end);
    assert ((*iter).second == map1);
    assert ((*iter).first.scalar() == std::string("foo"));
    ++iter;
    assert (iter == end);

    return 0;

}