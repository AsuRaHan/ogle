# API документация для скриптового движка

В проекте **OGLE** скрипты пишутся на JavaScript и работают через публичный API, доступный в глобальном объекте `ogle`. Ниже описаны функции, которые реально доступны в текущей реализации.

## Общие сведения
- Скрипты загружаются через `ScriptManager` (см. `src/App.cpp`). По умолчанию запускается файл, указанный в `app_config.json → scripts.startupScriptPath`.
- Внутри скрипта доступен глобальный объект `ogle`, через который вызываются функции движка.
- C++ классы можно экспортировать в JavaScript через `ScriptEngine::RegisterConstructor` и `RegisterMethod`.
- Вся работа с API выполняется в одном потоке движка, поэтому дополнительных механизмов синхронизации в JS обычно не требуется.

## Доступные пространства имён
| Имя | Описание |
| --- | --- |
| `ogle.world` | Создание и управление объектами мира (кубы, сферы, источники света). |
| `ogle.entity` | Получение и изменение состояния сущностей: позиция, вращение, проверка существования, имя. |
| `ogle.material` | Настройка материала объектов: цвет, эмиссия, шероховатость, металличность, отражение. |
| `ogle.light` | Изменение параметров освещения. |
| `ogle.physics` | Физика тел и коллизии. |
| `ogle.log` | Логирование сообщений в консоль движка. |

## `ogle.world`
| Функция | Параметры | Описание |
| --- | --- | --- |
| `clear()` | нет | Очищает мир от всех сущностей. |
| `createCube(options)` | `{name, position, scale}` | Создаёт куб и возвращает ID сущности. |
| `createSphere(options)` | `{name, position, radius}` | Создаёт сферу и возвращает ID сущности. |
| `createDirectionalLight(options)` | `{name, rotation, color, intensity, castShadows, primary}` | Создаёт направленный свет. |
| `createPointLight(options)` | `{name, position, color, intensity, range}` | Создаёт точечный свет. |

## `ogle.entity`
| Функция | Параметры | Описание |
| --- | --- | --- |
| `exists(id)` | `id` | Возвращает `true`, если сущность существует. |
| `getPosition(id)` | `id` | Возвращает `{x,y,z}` позицию сущности. |
| `setPosition(id, vec3)` | `id`, `{x,y,z}` | Устанавливает позицию сущности. |
| `getRotation(id)` | `id` | Возвращает `{x,y,z}` поворот сущности. |
| `setRotation(id, vec3)` | `id`, `{x,y,z}` | Устанавливает поворот сущности. |
| `getName(id)` | `id` | Возвращает имя сущности. |

## `ogle.material`
| Функция | Параметры | Описание |
| --- | --- | --- |
| `setBaseColor(id, color)` | `id`, `{x,y,z}` | Устанавливает основной цвет материала. |
| `setEmissiveColor(id, color)` | `id`, `{x,y,z}` | Устанавливает эмиссионный цвет. |
| `setRoughness(id, value)` | `id`, число | Устанавливает шероховатость материала. |
| `setMetallic(id, value)` | `id`, число | Устанавливает металличность материала. |
| `setReflectivity(id, value)` | `id`, число | Устанавливает отражательность материала. |

## `ogle.light`
| Функция | Параметры | Описание |
| --- | --- | --- |
| `setIntensity(id, value)` | `id`, число | Устанавливает интенсивность света. |

## `ogle.physics`
| Функция | Параметры | Описание |
| --- | --- | --- |
| `addBox(id, halfExtents, bodyType, mass)` | `id`, `{x,y,z}`, строка, число | Добавляет физическое тело коробки. |
| `setCollisionCallback(fn)` | `fn(entityA, entityB)` | Устанавливает JS-коллбэк на столкновения. |

## `ogle.log`
| Функция | Параметры | Описание |
| --- | --- | --- |
| `ogle.log(message)` | строка | Печатает сообщение в лог движка. |

## Пример скрипта
```js
var elapsedTime = 0;
var cubeId = null;

function onStart() {
    cubeId = ogle.world.createCube({
        name: "DemoCube",
        position: { x: 0, y: 1, z: 0 },
        scale: { x: 1, y: 1, z: 1 }
    });
    ogle.material.setBaseColor(cubeId, { x: 0.4, y: 0.7, z: 1.0 });
    ogle.physics.addBox(cubeId, { x: 0.5, y: 0.5, z: 0.5 }, "Dynamic", 1.0);
    ogle.world.createPointLight({
        name: "Lamp",
        position: { x: 2, y: 3, z: 0 },
        color: { x: 1.0, y: 0.8, z: 0.6 },
        intensity: 2.5,
        range: 8.0
    });
    ogle.physics.setCollisionCallback(function (entityA, entityB) {
        if (ogle.entity.exists(entityA) && ogle.entity.exists(entityB)) {
            ogle.log("Collision: " + ogle.entity.getName(entityA) + " vs " + ogle.entity.getName(entityB));
        }
    });
}

function onUpdate(dt) {
    elapsedTime += dt;
    if (ogle.entity.exists(cubeId)) {
        ogle.entity.setPosition(cubeId, {
            x: 0,
            y: 1 + Math.sin(elapsedTime * 2.0) * 0.5,
            z: 0
        });
    }
}
```

## Как добавить новый класс на C++ и сделать его доступным из скриптов
1. Создайте класс в C++, например `ScoreTracker`.

```cpp
#pragma once

namespace OGLE {
    class ScoreTracker {
    public:
        explicit ScoreTracker(int initialScore)
            : m_score(initialScore) {}

        int GetScore() const { return m_score; }
        void AddPoints(int points) { m_score += points; }

    private:
        int m_score;
    };
}
```

2. Зарегистрируйте конструктор и методы в `src/scripting/ScriptBindings.cpp`.

```cpp
#include "scripting/ScoreTracker.h"

engine.RegisterConstructor<ScoreTracker, int>("ScoreTracker");
engine.RegisterMethod<ScoreTracker>("getScore", &ScoreTracker::GetScore);
engine.RegisterMethod<ScoreTracker>("addPoints", &ScoreTracker::AddPoints);
```

3. Используйте класс в JS.

```js
function onStart() {
    const tracker = new ScoreTracker(100);
    ogle.log("Score = " + tracker.getScore());
    tracker.addPoints(50);
    ogle.log("After bonus: " + tracker.getScore());
}
```

> Примечание: конструктор регистрируется глобально в JavaScript. Если нужно, чтобы класс был доступен как `ogle.ScoreTracker`, добавьте в JS:
>
> ```js
> ogle.ScoreTracker = ScoreTracker;
> ```

## Как подключить свой скрипт
1. Создайте файл `assets/scripts/<имя>.js`.
2. Укажите его в `app_config.json` в поле `scripts.startupScriptPath`.
3. Перезапустите приложение.

## Текущий API
- `ogle.world`: `clear`, `createCube`, `createSphere`, `createDirectionalLight`, `createPointLight`
- `ogle.entity`: `exists`, `getPosition`, `setPosition`, `getRotation`, `setRotation`, `getName`
- `ogle.material`: `setBaseColor`, `setEmissiveColor`, `setRoughness`, `setMetallic`, `setReflectivity`
- `ogle.light`: `setIntensity`
- `ogle.physics`: `addBox`, `setCollisionCallback`
- `ogle.log`
