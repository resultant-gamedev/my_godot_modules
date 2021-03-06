//
// Created by gen on 15-5-24.
//

#include "action.h"
#include "../../behaviornode/linkerbnode.h"
#include "../../../scene/animation/animation_player.h"
#include "../anim_controller.h"
#include "../../../core/math/math_defs.h"
#include "../../../core/math/math_2d.h"

void Action::refresh_cancel_list() {
    if (!get_tree()->is_editor_hint()) {
        cancel_list.clear();
        int t = get_child_count(), off = 0;
        CancelNode** nodes = (CancelNode**)memalloc(t*sizeof(CancelNode*));
        for (int i = 0; i < t; ++i) {
            CancelNode* cancelNode = get_child(i)->cast_to<CancelNode>();
            if (cancelNode) {
                nodes[off] = cancelNode;
                off += 1;
                if (cancelNode->get_link_target()) {
                    Action *action = cancelNode->get_link_target()->cast_to<Action>();
                    if (action) {
                        CancelItem item;
                        item.time = cancelNode->get_cancel_time();
                        item.type = cancelNode->get_cancel_type();
                        item.action = action;
                        cancel_list.push_back(item);
                    }
                }
            }
        }
        for (int j = 0; j < off; ++j) {
            remove_child(nodes[j]);
        }
        memfree(nodes);
        checked_cancel_list = true;
    }
}

void Action::_notification(int p_notification) {
    switch (p_notification) {
        case NOTIFICATION_ENTER_TREE: {
            update_animation_path();
            update_next_action();

            break;
        }
    }
}

bool Action::_pre_behavior(const Variant &target, Dictionary env) {
    if (_allow) {
        return (bool)call("_check_action", target, env);
    }else {
        return false;
    }
}

BehaviorNode::Status Action::_step(const Variant& target, Dictionary &env) {
    if (!checked_cancel_list) {
        refresh_cancel_list();
    }
    if (_time <= 0 && timeout && force_enter) {
        force_enter = false;
        Status childrenStatus =  _traversal_children(target, env);
        Status status = (Status)((int)call(StringName("behavior"),target, Variant(env)));
        if (status == STATUS_DEPEND_ON_CHILDREN)
            return childrenStatus;
        else
            return status;
    }else {
        return TimerBNode::_step(target, env);
    }
}

void Action::_during_behavior(const Variant &target, Dictionary &env) {
    Vector2 v2 = env["move"];
    float abs_x = Math::abs(v2.x);
    if (abs_x > max_move) {
        if (v2.x > 0) {
            if (old_move > max_move) {
                v2.x = (old_move-drag) > max_move ? (old_move-drag):max_move;
            }else {
                v2.x = max_move;
            }
        }else {
            if (old_move < -max_move) {
                v2.x = (old_move+drag) < -max_move ? (old_move+drag):-max_move;
            }else {
                v2.x = -max_move;
            }
        }
    }
    env["move"] = v2;
    old_move = v2.x;

#define setforce(NODE)  {NODE->reset(target);\
    NODE->set_focus();\
    NODE->force_enter = true;\
    cancel();\
    reset(target);\
    return;}

    for (int i = 0, t = cancel_list.size(); i < t; ++i) {
        CancelItem& item = cancel_list[i];
        switch (item.type) {
            case CancelNode::HIT: {
                if (_is_hit && (bool)item.action->call("_check_action", target, env))
                    setforce(item.action);
            } break;
            case CancelNode::TIME: {
                if (get_delay()-get_time() > item.time && (bool)item.action->call("_check_action", target, env))
                    setforce(item.action);
            } break;
        }
    }
#undef setforce
}

void Action::set_animation_path(NodePath path) {
    animation_path = path;
    update_animation_path();
}

void Action::update_animation_path() {
    if (is_inside_tree() && animation_path != NodePath() && has_node(animation_path)) {
        animation_node = get_node(animation_path);
    }else {
        animation_node = NULL;
    }
}

void Action::update_next_action() {
    if (is_inside_tree() && next_action_path != NodePath() && has_node(next_action_path)) {
        next_action = get_node(next_action_path)->cast_to<Action>();
    }else {
        next_action = NULL;
    }
}

BehaviorNode::Status Action::_behavior(const Variant& target, Dictionary env) {
    TimerBNode::_behavior(target, env);
    old_move = ((Vector2)env["move"]).x;
    _is_hit = false;
    if (animation_node) {
        AnimController *manager = animation_node->cast_to<AnimController>();
        if (manager) {
            if (animation_type != "" && animation_name != "")
                manager->set_status(animation_type, animation_name);
        }else {
            AnimationPlayer *player = animation_node->cast_to<AnimationPlayer>();
            if (player && animation_name != "")
                player->play(animation_name);
        }
    }
    return BehaviorNode::STATUS_RUNNING;
}

void Action::cancel_animation() {
    if (animation_node) {
        AnimController *manager = animation_node->cast_to<AnimController>();
        if (manager) {
            if (animation_type != "" && animation_name != "") {
                manager->remove_status_with(animation_type, animation_name);
            }
        }
    }
}

void Action::_reset(const Variant &target) {
    TimerBNode::_reset(target);
    cancel_animation();
}

void Action::_timeout_behavior(const Variant& target, Dictionary& env) {
    cancel_animation();
    if (next_action) {
        time_out();
        next_action->reset(target);
        next_action->force_enter = true;
        next_action->set_focus();
    }
}

void Action::_cancel_behavior(const Variant& target, Dictionary& env) {
    cancel_animation();
}

void Action::_bind_methods() {
    ObjectTypeDB::bind_method(_MD("refresh_cancel_list"), &Action::refresh_cancel_list);

    ObjectTypeDB::bind_method(_MD("set_hit", "hit"), &Action::set_hit);
    ObjectTypeDB::bind_method(_MD("get_hit"), &Action::get_hit);

    ObjectTypeDB::bind_method(_MD("set_max_move", "max_move"), &Action::set_max_move);
    ObjectTypeDB::bind_method(_MD("get_max_move"), &Action::get_max_move);

    ObjectTypeDB::bind_method(_MD("set_drag", "drag"), &Action::set_drag);
    ObjectTypeDB::bind_method(_MD("get_drag"), &Action::get_drag);

    ObjectTypeDB::bind_method(_MD("set_animation_path", "animation_path"), &Action::set_animation_path);
    ObjectTypeDB::bind_method(_MD("get_animation_path"), &Action::get_animation_path);

    ObjectTypeDB::bind_method(_MD("get_animation_node"), &Action::get_animation_node);

    ObjectTypeDB::bind_method(_MD("set_animation_type", "animation_type"), &Action::set_animation_type);
    ObjectTypeDB::bind_method(_MD("get_animation_type"), &Action::get_animation_type);

    ObjectTypeDB::bind_method(_MD("set_animation_name", "animation_name"), &Action::set_animation_name);
    ObjectTypeDB::bind_method(_MD("get_animation_name"), &Action::get_animation_name);

    ObjectTypeDB::bind_method(_MD("set_next_action_path", "naxt_action_path"), &Action::set_next_action_path);
    ObjectTypeDB::bind_method(_MD("get_next_action_path"), &Action::get_next_action_path);


    ObjectTypeDB::bind_method(_MD("set_allow", "allow"), &Action::set_allow);
    ObjectTypeDB::bind_method(_MD("get_allow"), &Action::get_allow);

    ObjectTypeDB::bind_method(_MD("get_next_action"), &Action::get_next_action);

    ObjectTypeDB::bind_method(_MD("_check_action", "target", "env:Dictionary"), &Action::_check_action);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow"), _SCS("set_allow"), _SCS("get_allow"));

    ADD_PROPERTY(PropertyInfo(Variant::REAL, "move/max_move"), _SCS("set_max_move"), _SCS("get_max_move"));
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "move/drag"), _SCS("set_drag"), _SCS("get_drag"));

    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "action/animation_path"), _SCS("set_animation_path"), _SCS("get_animation_path"));
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "action/animation_type"), _SCS("set_animation_type"), _SCS("get_animation_type"));
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "action/animation_name"), _SCS("set_animation_name"), _SCS("get_animation_name"));

    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "next_action_path"), _SCS("set_next_action_path"), _SCS("get_next_action_path"));

    BIND_VMETHOD( MethodInfo("_cancel_behavior", PropertyInfo(Variant::OBJECT,"target")) );
    BIND_VMETHOD( MethodInfo("_check_action", PropertyInfo(Variant::OBJECT,"target"), PropertyInfo(Variant::DICTIONARY, "env")) );
}
