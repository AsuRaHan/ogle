# API документация для скриптового движка

В проекте **OGLE** скрипты пишутся на JavaScript и работают через публичный API, который доступен в глобальном объекте `ogle`.

Скрипты загружаются из `ScriptManager` и обычно стартуют из файла, указанного в `app_config.json → scripts.startupScriptPath`.

> В текущей реализации доступны следующие объекты: `world`, `entity`, `physics`, `log` и `Player`.
> При этом все они также доступны внутри `ogle`: `ogle.world`, `ogle.entity`, `ogle.physics`, `ogle.log`, `ogle.Player`.

## Доступные объекты

### `ogle.world`
API для создания сущностей и света.

| Функция | Параметры | Описание |
| --- | --- | --- |
| `clear()` | нет | Очищает весь мир. |
| `createCube(name, position, scale)` | `name: string`, `position: [x,y,z]`, `scale: [x,y,z]` | Создаёт куб и возвращает ID сущности. |
| `createSphere(name, position, radius)` | `name: string`, `position: [x,y,z]`, `radius: number` | Создаёт сферу и возвращает ID сущности. |
| `createDirectionalLight(name, rotation, color, intensity, castShadows, primary)` | `name: string`, `rotation: [x,y,z]`, `color: [r,g,b]`, `intensity: number`, `castShadows: boolean`, `primary: boolean` | Добавляет направленный источник света. |
| `createPointLight(name, position, color, intensity, range)` | `name: string`, `position: [x,y,z]`, `color: [r,g,b]`, `intensity: number`, `range: number` | Добавляет точечный источник света. |

### `ogle.entity`
API для чтения и изменения состояния сущности.

| Функция | Параметры | Описание |
| --- | --- | --- |
| `exists(entityId)` | `entityId: number` | Возвращает `true`, если сущность существует. |
| `getPosition(entityId)` | `entityId: number` | Возвращает `[x,y,z]` позицию сущности. |
| `setPosition(entityId, position)` | `entityId: number`, `position: [x,y,z]` | Устанавливает позицию сущности. |
| `getRotation(entityId)` | `entityId: number` | Возвращает `[x,y,z]` вращение сущности. |
| `setRotation(entityId, rotation)` | `entityId: number`, `rotation: [x,y,z]` | Устанавливает вращение сущности. |
| `getName(entityId)` | `entityId: number` | Возвращает имя сущности. |

### `ogle.physics`
API для базовой физики.

| Функция | Параметры | Описание |
| --- | --- | --- |
| `addBox(entityId, halfExtents, bodyType, mass)` | `entityId: number`, `halfExtents: [x,y,z]`, `bodyType: string`, `mass: number` | Добавляет физическое тело коробки к сущности. |

### `ogle.input`
API для опроса состояния ввода.

| Функция | Параметры | Описание |
| --- | --- | --- |
| `isKeyDown(key)` | `key: number` | Возвращает `true`, если клавиша удерживается. |
| `isKeyPressed(key)` | `key: number` | Возвращает `true`, если клавиша была нажата в этом кадре. |
| `isKeyReleased(key)` | `key: number` | Возвращает `true`, если клавиша была отпущена в этом кадре. |
| `getMousePosition()` | нет | Возвращает позицию мыши `[x,y]`. |
| `getMouseDelta()` | нет | Возвращает движение мыши за кадр `[dx,dy]`. |
| `getMouseWheelDelta()` | нет | Возвращает прокрутку колесика мыши. |
| `isMouseButtonDown(button)` | `button: number` | Возвращает `true`, если кнопка мыши удерживается. |
| `isMouseButtonPressed(button)` | `button: number` | Возвращает `true`, если кнопка мыши была нажата в этом кадре. |
| `isMouseButtonReleased(button)` | `button: number` | Возвращает `true`, если кнопка мыши была отпущена в этом кадре. |
| `isGamepadConnected(player)` | `player: number` | Возвращает `true`, если геймпад подключён. |
| `isGamepadButtonDown(player, button)` | `player: number`, `button: number` | Возвращает `true`, если кнопка геймпада удерживается. |
| `getGamepadAxis(player, axis)` | `player: number`, `axis: number` | Возвращает значение оси геймпада. |
| `getAxis(name)` | `name: string` | Возвращает текущее значение оси. |
| `getAxisRaw(name)` | `name: string` | Возвращает сырое значение оси. |
| `getCurrentContext()` | нет | Возвращает текущее имя контекста ввода. |

### `ogle.log`
Логирование сообщений в движок.

| Функция | Параметры | Описание |
| --- | --- | --- |
| `log(message)` | `message: string` | Выводит сообщение в лог. |

### `ogle.Player`
Класс `Player` экспортируется через dukglue и доступен глобально.

| Конструктор / метод | Описание |
| --- | --- |
| `new ogle.Player(initialHp)` | Создаёт объект игрока. |
| `getHP()` | Возвращает текущее здоровье. |
| `takeDamage(amount)` | Наносит урон игроку. |

## Работа с JS-массивами
В текущем API позиция, вращение, цвет и размеры передаются как массивы чисел, например:

```js
ogle.entity.setPosition(entityId, [0, 1, 0]);
ogle.world.createPointLight("Lamp", [2, 3, 0], [1.0, 0.8, 0.6], 2.5, 8.0);
```

## Пример скрипта
```js
var cubeId = null;

function onStart() {
    cubeId = ogle.world.createCube("DemoCube", [0, 1, 0], [1, 1, 1]);
    ogle.physics.addBox(cubeId, [0.5, 0.5, 0.5], "Dynamic", 1.0);
    ogle.world.createPointLight("Lamp", [2, 3, 0], [1.0, 0.8, 0.6], 2.5, 8.0);
    ogle.log("Cube created: " + ogle.entity.getName(cubeId));
}

function onUpdate(dt) {
    if (ogle.entity.exists(cubeId)) {
        var pos = ogle.entity.getPosition(cubeId);
        pos[1] = 1 + Math.sin(dt * 2.0) * 0.5;
        ogle.entity.setPosition(cubeId, pos);
    }
}
```

## Добавление нового C++ API в JS
Если нужно экспортировать новый класс из C++, зарегистрируйте его в `src/scripting/ScriptBindings.cpp` с помощью `ScriptEngine::RegisterConstructor` и `ScriptEngine::RegisterMethod`.

Пример:

```cpp
engine.RegisterConstructor<ScoreTracker, int>("ScoreTracker");
engine.RegisterMethod<ScoreTracker>("getScore", &ScoreTracker::GetScore);
engine.RegisterMethod<ScoreTracker>("addPoints", &ScoreTracker::AddPoints);
```

В JavaScript класс будет доступен глобально:

```js
var tracker = new ScoreTracker(100);
ogle.log(tracker.getScore());
```

## Как подключить скрипт
1. Создайте файл `assets/scripts/<имя>.js`.
2. Укажите его в `app_config.json` в поле `scripts.startupScriptPath`.
3. Перезапустите приложение.

## Текущий API
- `ogle.world`: `clear`, `createCube`, `createSphere`, `createDirectionalLight`, `createPointLight`
- `ogle.entity`: `exists`, `getPosition`, `setPosition`, `getRotation`, `setRotation`, `getName`
- `ogle.physics`: `addBox`
- `ogle.log`: `log`
- `ogle.Player`: `new ogle.Player(initialHp)`, `getHP()`, `takeDamage(amount)`
