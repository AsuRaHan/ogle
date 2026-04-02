#pragma once

#include <entt/entt.hpp>
#include <glm/vec3.hpp>

#include "../render/Material.h"

#include <memory>
#include <string>

namespace OGLE {
    class ModelEntity;

    // Перечисление, определяющее тип объекта в мире.
    // Используется для быстрой идентификации типа сущности.
    enum class WorldObjectKind {
        Generic,    // Общий тип по умолчанию
        Mesh,       // Объект с 3D-моделью
        Light,      // Источник света
        Billboard   // Плоский объект, всегда повернутый к камере
    };

    // Базовый компонент, описывающий состояние любого объекта в мире.
    struct WorldObjectComponent {
        WorldObjectKind kind = WorldObjectKind::Generic; // Тип объекта
        bool enabled = true;     // Включен ли объект (влияет на логику и обновление)
        bool visible = true;     // Виден ли объект (влияет на рендеринг)
    };

    // Компонент для хранения имени сущности.
    struct NameComponent {
        std::string value; // Строка с именем
    };

    // Компонент, отвечающий за положение, вращение и масштаб объекта в 3D-пространстве.
    struct TransformComponent {
        glm::vec3 position{ 0.0f, 0.0f, 0.0f }; // Позиция в мире
        glm::vec3 rotation{ 0.0f, 0.0f, 0.0f }; // Углы Эйлера (вращение)
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };    // Масштаб
    };

    // Компонент, содержащий ссылку на 3D-модель (меш) объекта.
    struct ModelComponent {
        std::shared_ptr<ModelEntity> model; // Умный указатель на ресурс модели
    };

    // Перечисление для определения исходного типа геометрии объекта.
    enum class PrimitiveType {
        None,           // Нет примитива
        Cube,           // Куб
        Sphere,         // Сфера
        Plane,          // Плоскость
        // Cylinder,       // Цилиндр
        // Cone,           // Конус
        ModelFile,      // Модель из файла
        ProceduralMesh  // Процедурно сгенерированный меш
    };

    // Компонент, описывающий "исходник" геометрии сущности.
    // Полезен для редактора и сериализации, чтобы знать, как объект был создан.
    struct PrimitiveComponent {
        PrimitiveType type = PrimitiveType::None; // Тип примитива
        std::string sourcePath;                   // Путь к исходному файлу (например, для модели)
    };

    // Компонент, содержащий данные о материале объекта.
    // Вынесен отдельно от модели, чтобы материалы можно было переиспользовать.
    struct MaterialComponent {
        Material material; // Структура с параметрами материала (цвет, текстуры и т.д.)
    };

    // Типы источников света.
    enum class LightType {
        Directional, // Направленный (как солнце)
        Point        // Точечный
    };

    // Компонент для сущностей, являющихся источниками света.
    struct LightComponent {
        LightType type = LightType::Directional;  // Тип света
        glm::vec3 color{ 1.0f, 1.0f, 1.0f };       // Цвет света
        float intensity = 1.0f;                   // Интенсивность
        float range = 10.0f;                      // Радиус действия для точечных источников
        bool castShadows = false;                 // Отбрасывает ли тень
        bool primary = false;                     // Является ли основным источником света в сцене
    };

    // Компонент для хранения данных о скелете модели.
    // Пока что заготовка для будущей поддержки скиннинга и анимации.
    struct SkeletonComponent {
        bool enabled = false;      // Включен ли скелет
        int boneCount = 0;         // Количество костей
        std::string sourcePath;    // Путь к файлу, откуда был загружен скелет
    };

    // Компонент для управления состоянием анимации.
    // Хранится отдельно от модели, чтобы можно было гибко управлять проигрыванием.
    struct AnimationComponent {
        bool enabled = false;        // Включена ли анимация
        bool playing = false;        // Проигрывается ли сейчас
        bool loop = true;            // Зациклена ли анимация
        float currentTime = 0.0f;    // Текущее время проигрывания
        float playbackSpeed = 1.0f;  // Скорость воспроизведения
        std::string currentClip;     // Название текущего анимационного клипа
    };

    // Компонент для привязки скрипта к сущности.
    // Хранит только информацию о том, какой скрипт использовать.
    struct ScriptComponent {
        bool enabled = true;          // Включен ли скрипт
        bool autoStart = false;       // Запускать ли скрипт автоматически при старте сцены
        std::string scriptPath;       // Путь к файлу скрипта
    };

    // Типы физических тел.
    enum class PhysicsBodyType {
        Static,    // Статичное (неподвижное)
        Dynamic,   // Динамическое (реагирует на силы)
        Kinematic  // Кинематическое (управляется вручную, но влияет на другие)
    };

    // Компонент для описания физического тела объекта.
    struct PhysicsBodyComponent {
        PhysicsBodyType type = PhysicsBodyType::Static; // Тип тела
        float mass = 0.0f;                              // Масса (для динамических тел)
        glm::vec3 halfExtents{ 0.5f, 0.5f, 0.5f };      // Полуразмеры (для примитивных форм вроде куба)
        bool simulate = true;                           // Включена ли симуляция для этого тела
    };

    // Псевдоним для типа сущности из библиотеки EnTT для удобства.
    using Entity = entt::entity;
}
