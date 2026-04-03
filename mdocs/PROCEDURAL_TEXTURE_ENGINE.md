# Процедурная генерация текстур (Procedural Texture Engine)

Движок для создания текстур на основе алгоритмов шумов Перлина/Симплекса, как в демо-сценах 💎

## Особенности

- **GPU-ускоренная генерация** через compute shaders
- **Множество алгоритмов**: Перлин, FBM, мрамор, дерево, облака, Вороной, горы
- **Полный контроль параметров**: масштаб, октавы, персистентность, лакунарность
- **Удобный API** через TextureGenerator
- **Интеграция с системой материалов**

## Виды текстур

### 1. Облака (Clouds)
```cpp
auto cloudTexture = TextureGenerator::GenerateClouds(
    512, 512,           // размер
    2.0f,               // масштаб
    6,                  // октавы
    0.5f,               // persistence
    2.0f,               // lacunarity
    42                  // seed
);
```

### 2. Мрамор (Marble)
```cpp
auto marbleTexture = TextureGenerator::GenerateMarble(
    512, 512,
    2.0f,
    4,
    glm::vec3(0.2f),    // тёмный цвет
    glm::vec3(0.9f)     // светлый цвет
);
```

### 3. Дерево (Wood)
```cpp
auto woodTexture = TextureGenerator::GenerateWood(
    512, 512,
    1.5f,
    3,
    glm::vec3(0.3f, 0.2f, 0.1f),   // тёмный коричневый
    glm::vec3(0.6f, 0.4f, 0.2f)    // светлый коричневый
);
```

### 4. Горы (RidgedNoise)
```cpp
auto mountainTexture = TextureGenerator::GenerateMountains(
    512, 512,
    3.0f,
    8,  // больше октав = больше деталей
    glm::vec3(0.1f, 0.2f, 0.3f),  // синий
    glm::vec3(0.9f)                 // белый (снег)
);
```

### 5. Шахматная доска (Checkerboard)
```cpp
auto checkerTexture = TextureGenerator::GenerateCheckerboard(
    512, 512,
    32.0f,  // размер одного квадрата
    glm::vec3(0.0f),
    glm::vec3(1.0f)
);
```

### 6. Диаграмма Вороного (Voronoi)
```cpp
auto voronoiTexture = TextureGenerator::GenerateVoronoi(
    512, 512,
    5.0f
);
```

### 7. Перлин шум (базовый)
```cpp
auto perlTexture = TextureGenerator::GeneratePerlinNoise(
    512, 512,
    2.0f
);
```

### 8. FBM (Full control)
```cpp
auto fbmTexture = TextureGenerator::GenerateFBM(
    512, 512,
    2.0f,      // масштаб
    4,         // октавы - чем больше, тем больше деталей
    0.5f,      // persistence - вес каждой октавы (0-1)
    2.0f       // lacunarity - частотный множитель между октавами
);
```

### 9. Турбулентность (Turbulence)
```cpp
auto turbTexture = TextureGenerator::GenerateTurbulence(
    512, 512,
    2.0f,
    4,
    glm::vec3(0.0f, 0.0f, 1.0f),  // холодный (синий)
    glm::vec3(1.0f, 0.0f, 0.0f)   // горячий (красный)
);
```

## Параметры для экспериментов

### Масштаб (scale)
- **0.5-1.0**: большие мягкие формы
- **1.0-3.0**: нормальный размер деталей
- **3.0+**: много мелких деталей

### Октавы (octaves)
- **1**: простой шум
- **2-4**: умеренно сложный
- **6-8**: очень подробное

### Персистентность (persistence)
- **0.3**: мягкие плавные переходы
- **0.5**: балансированный вид
- **0.7+**: более контрастные шумы

### Лакунарность (lacunarity)
- **1.5**: сжатая частота
- **2.0**: стандартная (демо-сценский стандарт)
- **3.0+**: редкие детали большого масштаба

## Использование в коде

### Инициализация при запуске
```cpp
// В файле, где инициализируется ShaderManager
#include "render/ProceduralTexture.h"

ShaderManager shaderManager;
// ... обычная инициализация ...

// После загрузки обычных shader'ов
ProceduralTexture::InitializeShaders(&shaderManager);
```

### Применение к материалу
```cpp
#include "render/TextureGenerator.h"

// Создаём процедурную текстуру
auto texture = TextureGenerator::GenerateClouds();

// Используем в материале
material.SetDiffuseTexture(texture);

// Или просто привязываем
glBindTexture(GL_TEXTURE_2D, texture->GetTextureId());
```

### Сохранение / Повторное использование
```cpp
// Текстуры кэшируются по имени
// Повторное обращение не пересчитает текстуру
auto tex1 = TextureGenerator::GenerateMarble(512, 512, 2.0f, 4, glm::vec3(0), glm::vec3(1), 42);
auto tex2 = TextureGenerator::GenerateMarble(512, 512, 2.0f, 4, glm::vec3(0), glm::vec3(1), 42);
// tex1 и tex2 одно и то же!
```

## Типовые комбинации

### Земля/Грунт
```cpp
auto terrain = TextureGenerator::GenerateFBM(
    1024, 1024,
    3.0f, 6,
    glm::vec3(0.1f, 0.08f, 0.05f),  // тёмный грунт
    glm::vec3(0.4f, 0.35f, 0.25f)   // светлый грунт
);
```

### Металл/Ржавчина
```cpp
auto rustTexture = TextureGenerator::GenerateTurbulence(
    512, 512, 2.5f, 5,
    glm::vec3(0.3f, 0.3f, 0.35f),  // серый металл
    glm::vec3(0.8f, 0.4f, 0.1f)    // ржавый оранж
);
```

### Ткань/Канва
```cpp
auto fabricTexture = TextureGenerator::GenerateCheckerboard(
    256, 256,
    8.0f,
    glm::vec3(0.9f),
    glm::vec3(0.7f)
);
```

## Типы параметров в методах

- `width, height`: размер текстуры в пикселях (обычно степени двойки: 256, 512, 1024)
- `scale`: масштабирование координат шума
- `octaves`: количество слоёв шума
- `persistence`: влияние высоких частот (0-1)
- `lacunarity`: множитель частоты между слоями
- `color1, color2`: цвета интерполяции (RGB от 0 до 1)
- `seed`: число для инициализации PRNG (для разных нужны разные значения)

## Примечания

- Compute shaders работают на GPU → очень быстро даже для больших текстур
- Генерация один раз при инициализации = нулевой overhead при рендере
- Полностью совместимо с существующей системой материалов
- Можно комбинировать процедурные текстуры с обычными в одном материале
