#include "knoting/shooting_subsystem.h"
#include <knoting/components.h>
#include <knoting/engine.h>
#include <knoting/input_manager.h>
#include <knoting/scene.h>

namespace knot {
Shooting::Shooting(Engine& engine) : m_engine(engine), m_inManager(nullptr) {}

void Shooting::on_awake() {
    m_inManager = m_engine.get_window_module().lock()->get_input_manager();
}

void Shooting::on_update(double m_delta_time) {
    auto sceneOpt = Scene::get_active_scene();
    if (!sceneOpt) {
        return;
    }
    Scene& scene = sceneOpt.value();
    entt::registry& registry = scene.get_registry();

    if (bullSize > 100) {
        auto go = bullets.front();
        scene.remove_game_object(go);
        bullets.pop_front();
        bullSize--;
    }

    auto players = registry.view<components::Transform, components::Health, components::Tag>();
    for (auto& player : players) {
        auto goOpt = scene.get_game_object_from_handle(player);
        if (!goOpt) {
            continue;
        }

        GameObject go = goOpt.value();
        auto& transform = go.get_component<components::Transform>();
        auto& health = go.get_component<components::Health>();

        auto tag = go.get_component<components::Tag>();
        if (tag.get_id() != tag.get_id_from_tag("PLAYER")) {
            continue;
        }

        // shooting
        if (m_inManager->mouse_button_pressed(MouseButtonCode::Left)) {
            health.set_health(health.get_health() - 1);

            vec3 spawnPos = transform.get_position() + transform.forward();
            auto bullet = scene.create_cube("bullet", spawnPos, vec3(0), vec3(0.3f), true, 0.3f);
            bullets.emplace_back(bullet);
            bullSize++;
            auto bulController = bullet.get_component<components::RigidController>();
            bulController.add_force(transform.forward() * 1000.0f);

            if (health.get_health() < 0) {
                health.set_health(0);
            }
        }
        // refill paint
        if (m_inManager->key_held_down(KeyCode::E)) {
            health.set_health(health.get_health() + 1);

            if (health.get_health() > health.get_max_health()) {
                health.set_health(health.get_max_health());
            }
        }
    }
}

void Shooting::on_destroy() {}

}  // namespace knot

// TODO if shooting key pressed: shoot bullets