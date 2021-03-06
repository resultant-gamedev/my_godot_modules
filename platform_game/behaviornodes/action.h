//
// Created by gen on 15-5-24.
//

#ifndef GODOT_MASTER_ACTION_H
#define GODOT_MASTER_ACTION_H

#include "../../behaviornode/timebnode.h"
#include "../../../core/object.h"
#include "../../../core/reference.h"
#include "cancel_node.h"

class Action : public TimerBNode {
OBJ_TYPE(Action, TimerBNode);
private:
    struct CancelItem {
        int type;
        float time;
        Action *action;

        CancelItem() {
        }
    };

    Vector< CancelItem > cancel_list;
    bool checked_cancel_list;
    bool _is_hit;

    float max_move;
    float drag;
    float old_move;

    bool reset_from_cancel;

    Node*       animation_node;
    NodePath    animation_path;
    void update_animation_path();
    String      animation_type;
    String      animation_name;

    NodePath    next_action_path;
    Action      *next_action;
    void        update_next_action();

    void        cancel_animation();

    bool    _allow;
protected:
    void _notification(int p_notification);
    virtual bool    _pre_behavior(const Variant& target, Dictionary env);
    virtual Status  _step(const Variant& target, Dictionary &env);
    virtual void    _during_behavior(const Variant& target, Dictionary& env);
    virtual void    _timeout_behavior(const Variant& target, Dictionary& env);
    virtual void    _cancel_behavior(const Variant& target, Dictionary& env);
    virtual Status  _behavior(const Variant& target, Dictionary env);
    virtual void    _reset(const Variant& target);
    virtual bool    _check_action(const Variant& target, Dictionary env) { return false; }
    static void _bind_methods();
public:
    bool force_enter;
    void refresh_cancel_list();

    _FORCE_INLINE_ bool get_hit() {return _is_hit;}
    _FORCE_INLINE_ void set_hit(bool hit) {_is_hit = hit;}

    _FORCE_INLINE_ float get_max_move() {return max_move;}
    _FORCE_INLINE_ void set_max_move(float p_max_move) {if (p_max_move>=0)max_move=p_max_move;}

    _FORCE_INLINE_ float get_drag() {return drag;}
    _FORCE_INLINE_ void set_drag(float p_drag) {if (p_drag >= 0)drag=p_drag;}

    _FORCE_INLINE_ NodePath get_animation_path() {return animation_path;}
    void set_animation_path(NodePath path);

    _FORCE_INLINE_ Node *get_animation_node() {return animation_node;}

    _FORCE_INLINE_ String get_animation_type() {return animation_type;}
    _FORCE_INLINE_ void set_animation_type(String type) {animation_type=type;}

    _FORCE_INLINE_ String get_animation_name() {return animation_name;}
    _FORCE_INLINE_ void set_animation_name(String name) {animation_name = name;}

    _FORCE_INLINE_ void set_next_action_path(NodePath path) {next_action_path=path;update_next_action();}
    _FORCE_INLINE_ NodePath get_next_action_path() {return next_action_path;}

    _FORCE_INLINE_ void set_allow(bool p_allow) {_allow=p_allow;}
    _FORCE_INLINE_ bool get_allow() { return _allow;}

    _FORCE_INLINE_ Action *get_next_action() {return next_action;}

    Action() {
        checked_cancel_list= false;
        _is_hit= false;
        force_enter = false;
        animation_node=NULL;
        max_move=2;drag=0.2;
        reset_from_cancel = false;
        next_action = NULL;
        _allow = true;
    }
};


#endif //GODOT_MASTER_ACTION_H
