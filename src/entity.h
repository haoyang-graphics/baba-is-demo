#ifndef BABA_IS_YOU_ENTITY_H
#define BABA_IS_YOU_ENTITY_H

#include "words.h"
#include "property.h"
#include "level_controller.h"
#include "vector2.h"

#include <Godot.hpp>
#include <Node2D.hpp>
#include <nativescript/godot_nativescript.h>

#include <queue>
#include <unordered_map>

struct PropertyPriorityComparer {
    bool operator()(const Property *prop1, const Property *prop2) const {
        return prop1->get_priority() < prop2->get_priority();
    }
};

class Entity : public godot::Node2D {
GODOT_CLASS(Entity, Node2D)
public:
    std::priority_queue<Property *, std::vector<Property *>, PropertyPriorityComparer> properties;

    static void _register_methods() {
        godot::register_method("_enter_tree", &Entity::_enter_tree);
        godot::register_method("_exit_tree", &Entity::_exit_tree);
        godot::register_property<Entity, size_t>("Noun", &Entity::noun, 0,
                                                 GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_DEFAULT,
                                                 GODOT_PROPERTY_HINT_ENUM, kNounsHintString);
    }

    void _init() {
        update_tile_pos();
    }

    Nouns get_noun() const {
        return static_cast<Nouns>(noun + static_cast<size_t>(Nouns::ALGAE));
    }

    void set_noun(Nouns value) {
        noun = static_cast<size_t>(value) - static_cast<size_t>(Nouns::ALGAE);
    }

    TilePosition get_tile_pos() const {
        return tile_pos_;
    }

    void _enter_tree() {
        register_entity();
        LevelController::instance->controlledEntities.insert(this);// TODO: temporarily add to controlled entities for test
    }

    void _exit_tree() {
        unregister_entity();
        LevelController::instance->controlledEntities.erase(this);
    }

    void set_position(const godot::Vector2 value) {
        godot::Node2D::set_position(value);
        update_tile_pos();
    }

    void register_entity() {
        nounEntityMap[get_noun()].insert(this);
        posEntityMap[get_tile_pos()].insert(this);
    }

    void unregister_entity() {
        nounEntityMap[get_noun()].erase(this);
        posEntityMap[get_tile_pos()].erase(this);
    }

    void set_tile_pos(TilePosition newPos) {
        unregister_entity();
        auto tileSize = get_tile_size();
        auto pos = static_cast<godot::Vector2>(newPos) * tileSize + 0.5f * godot::Vector2(tileSize, tileSize);
        set_position(pos);
        register_entity();
    }

    template<class OutIter>
    static OutIter get_entities_at_pos(TilePosition pos, OutIter dest) {
        auto iter = posEntityMap.find(pos);
        if (iter == posEntityMap.cend()) return dest;
        return std::copy(iter->second.cbegin(), iter->second.cend(), dest);
    }

protected:
    size_t noun = 0;
    TilePosition tile_pos_;

    static std::unordered_map<Nouns, std::unordered_set<Entity *>> nounEntityMap;
    static std::unordered_map<TilePosition, std::unordered_set<Entity *>, HashVector2> posEntityMap;

    static real_t get_tile_size() {
        return LevelController::instance->get_tile_size();
    }

    void update_tile_pos() {
        auto tileSize = get_tile_size();
        auto tilePos = (get_position() - 0.5f * godot::Vector2(tileSize, tileSize)) / tileSize;
        tile_pos_ = TilePosition(std::lround(tilePos.x), std::lround(tilePos.y));
    }
};

#endif //BABA_IS_YOU_ENTITY_H
