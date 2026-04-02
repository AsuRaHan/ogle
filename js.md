# API документация для скриптового движка

В проекте **OGLE** скрипты пишутся на JavaScript и работают через публичный API, доступный в глобальном объекте `ogle`.  Ниже перечислены основные функции и пространства имён, которые можно использовать в пользовательских скриптах.

## Общие сведения
- Скрипты загружаются через `ScriptManager` (см. `src/App.cpp`).  По умолчанию запускается файл, указанный в `app_config.json` → `scripts.startupScriptPath`.
- Внутри скрипта можно обращаться к объекту `ogle` – это интерфейс к движку, который предоставляет доступ к миру, сущностям, материалам, источникам света и утилитам.
- Функции работают асинхронно в основном цикле приложения, но все операции с API выполняются в одной потоке, поэтому не требуется синхронизации.

## Пространства имён
| Имя | Описание |
|------|----------|
| `ogle.world` | Создание и управление объектами в мире (кубы, сферы, источники света и т.д.). |
| `ogle.entity` | Управление существующими сущностями: позиция, вращение, масштаб, проверка существования, удаление. |
| `ogle.material` | Установка базовых свойств материалов (цвет, отражённость и т.д.). |
| `ogle.light` | Управление точечными и направленными источниками света (интенсивность, цвет, диапазон). |
| `ogle.log` | Утилита для вывода логов в консоль движка. |
| `ogle.input` | (не используется в примере, но доступно) Получение и установка входных событий. |
| `ogle.math` | Базовые математические функции и типы (v3, quaternion). |

## Основные функции `ogle.world`
| Функция | Параметры | Возвращаемое значение |
|---------|-----------|------------------------|
| `createCube(params)` | `{name, position, scale}` – позиция и размер в виде объектов `{x,y,z}` | ID созданной сущности (целое число) |
| `createPointLight(params)` | `{name, position, color, intensity, range}` | ID источника света |
| `createSphere(params)` | `{name, position, radius}` | ID сферы |
| `createPlane(params)` | `{name, position, normal}` | ID плоскости |

## Управление сущностями `ogle.entity`
| Функция | Параметры | Пример |
|---------|-----------|--------|
| `setPosition(id, {x,y,z})` | ID, координаты | `ogle.entity.setPosition(entityId, {x:0,y:1.2,z:4})` |
| `setRotation(id, {x,y,z})` | ID, углы в градусах | `ogle.entity.setRotation(id, {x:0,y:45,z:0})` |
| `setScale(id, {x,y,z})` | ID, масштаб | `ogle.entity.setScale(id, {x:1.5,y:1.5,z:1.5})` |
| `exists(id)` | ID | `ogle.entity.exists(id)` → `true/false` |
| `delete(id)` | ID | Удаляет сущность из мира |

## Материалы `ogle.material`
| Функция | Параметры | Пример |
|---------|-----------|--------|
| `setBaseColor(id, {x,y,z})` | ID, цвет в диапазоне 0–1 | `ogle.material.setBaseColor(id, {x:0.9,y:0.95,z:1.0})` |
| `setReflectivity(id, value)` | ID, значение 0–1 | `ogle.material.setReflectivity(id, 0.3)` |

## Свет `ogle.light`
| Функция | Параметры | Пример |
|---------|-----------|--------|
| `setIntensity(id, value)` | ID, интенсивность | `ogle.light.setIntensity(lamp, 2.0)` |
| `setColor(id, {x,y,z})` | ID, цвет | `ogle.light.setColor(lamp, {x:1.0,y:0.7,z:0.35})` |

## Логирование `ogle.log`
| Функция | Параметры | Пример |
|---------|-----------|--------|
| `ogle.log(message)` | Строка | `ogle.log("Скрипт запущен")` |

## Пример скрипта (test_world.js)
```js
function onStart() {
    // Создаём куб
    const cube = ogle.world.createCube({name:"Cube", position:{x:-2.5,y:0.5,z:-2.5}, scale:{x:0.9,y:0.9,z:0.9}});
    // Устанавливаем цвет
    ogle.material.setBaseColor(cube, {x:0.9,y:0.95,z:1.0});
    // Лампочка
    const lamp = ogle.world.createPointLight({name:"Lamp", position:{x:-2.0,y:2.5,z:-2.0}, color:{x:1.0,y:0.7,z:0.35}, intensity:2.5, range:6.0});
}

function onUpdate(dt) {
    // Анимация куба
    const pos = {x:-2.5, y:0.6 + Math.sin(elapsedTime*2.0)*0.4, z:-2.5};
    ogle.entity.setPosition(animatedCube, pos);
    ogle.entity.setRotation(animatedCube, 0.0, elapsedTime*90.0, 0.0);
    // Изменяем интенсивность лампы
    if (ogle.entity.exists(lamp)) {
        ogle.light.setIntensity(lamp, 2.0 + Math.sin(elapsedTime*3.0)*0.5);
    }
}
```

## Как добавить новый скрипт
1. Создайте файл `my21крипты загружаются через `ScriptManager` (см. `src/App.cpp`).  По умолчанию запускается файл, указанный в `app_config.json` → `scripts.startupScriptPath`.
- Внутри скрипта можно обращаться к объекту `ogle` – это интерфейс к движку, который предоставляет доступ к миру, сущностям, материалам, источникам света и утилитам.
- Функции работают асинхронно в основном цикле приложения, но все операции с API выполняются в одной потоке, поэтому не требуется синхронизации.

## Пространства имён
| Имя | Описание |
|------|----------|
| `ogle.world` | Создание и управление объектами в мире (кубы, сферы, источники света и т.д.). |
| `ogle.entity` | Управление существующими сущностями: позиция, вращение, масштаб, проверка существования, удаление. |
| `ogle.material` | Установка базовых свойств материалов (цвет, отражённость и т.д.). |
| `ogle.light` | Управление точечными и направленными источниками света (интенсивность, цвет, диапазон). |
| `ogle.log` | Утилита для вывода логов в консоль движка. |
| `ogle.input` | (не используется в примере, но доступно) Получение и установка входных событий. |
| `ogle.math` | Базовые математические функции и типы (v3, quaternion). |

## Основные функции `ogle.world`
| Функция | Параметры | Возвращаемое значение |
|---------|-----------|------------------------|
| `createCube(params)` | `{name, position, scale}` – позиция и размер в виде объектов `{x,y,z}` | ID созданной сущности (целое число) |
| `createPointLight(params)` | `{name, position, color, intensity, range}` | ID источника света |
| `createSphere(params)` | `{name, position, radius}` | ID сферы |
| `createPlane(params)` | `{name, position, normal}` | ID плоскости |

## Управление сущностями `ogle.entity`
| Функция | Параметры | Пример |
|---------|-----------|--------|
| `setPosition(id, {x,y,z})` | ID, координаты | `ogle.entity.setPosition(entityId, {x:0,y:1.2,z:4})` |
| `setRotation(id, {x,y,z})` | ID, углы в градусах | `ogle.entity.setRotation(id, {x:0,y:45,z:0})` |
| `setScale(id, {x,y,z})` | ID, масштаб | `ogle.entity.setScale(id, {x:1.5,y:1.5,z:1.5})` |
| `exists(id)` | ID | `ogle.entity.exists(id)` → `true/false` |
| `delete(id)` | ID | Удаляет сущность из мира |

## Материалы `ogle.material`
| Функция | Параметры | Пример |
|---------|-----------|--------|
| `setBaseColor(id, {x,y,z})` | ID, цвет в диапазоне 0–1 | `ogle.material.setBaseColor(id, {x:0.9,y:0.95,z:1.0})` |
| `setReflectivity(id, value)` | ID, значение 0–1 | `ogle.material.setReflectivity(id, 0.3)` |

## Свет `ogle.light`
| Функция | Параметры | Пример |
|---------|-----------|--------|
| `setIntensity(id, value)` | ID, интенсивность | `ogle.light.setIntensity(lamp, 2.0)` |
| `setColor(id, {x,y,z})` | ID, цвет | `ogle.light.setColor(lamp, {x:1.0,y:0.7,z:0.35})` |

## Логирование `ogle.log`
| Функция | Параметры | Пример |
|---------|-----------|--------|
| `ogle.log(message)` | Строка | `ogle.log("Скрипт запущен")` |

## Пример скрипта (test_world.js)
```js
function onStart() {
    // Создаём куб
    const cube = ogle.world.createCube({name:"Cube", position:{x:-2.5,y:0.5,z:-2.5}, scale:{x:0.9,y:0.9,z:0.9}});
    // Устанавливаем цвет
    ogle.material.setBaseColor(cube, {x:0.9,y:0.95,z:1.0});
    // Лампочка
    const lamp = ogle.world.createPointLight({name:"Lamp", position:{x:-2.0,y:2.5,z:-2.0}, color:{x:1.0,y:0.7,z:0.35}, intensity:2.5, range:6.0});
}

function onUpdate(dt) {
    // Анимация куба
    const pos = {x:-2.5, y:0.6 + Math.sin(elapsedTime*2.0)*0.4, z:-2.5};
    ogle.entity.setPosition(animatedCube, pos);
    ogle.entity.setRotation(animatedCube, 0.0, elapsedTime*90.0, 0.0);
    // Изменяем интенсивность лампы
    if (ogle.entity.exists(lamp)) {
        ogle.light.setIntensity(lamp, 2.0 + Math.sin(elapsedTime*3.0)*0.5);
    }
}
```

## Как добавить новый скрипт
1. Создайте файл `my_script.js` в папке `assets/scripts`. 2. В `app_config.json` укажите путь к этому файлу в поле `scripts.startupScriptPath` (или запустите его вручную через консоль `ogle.runScript('assets/scripts/my_script.js')`). 3. Перезапустите приложение.

## FAQ
- **Можно ли использовать сторонние библиотеки JavaScript?** Да, можно подключать .js файлы как обычные скрипты, но они должны быть загружены до выполнения вашего скрипта.
- **Как получить ID созданной сущности?** Функции `create*` возвращают целочисленный ID. Можно сохранить его в переменную и использовать в дальнейшем.
- **Есть ли типы данных?** Внутри скриптов доступны объекты с полями `{x,y,z}`, `v3`, `quaternion` и простые типы (числа, строки, булевы).  Для более сложных структур лучше использовать C++ API.

---
Документация будет обновляться по мере добавления новых функций в движок.
