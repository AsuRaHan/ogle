# Editor Decomposition Plan

## Goal

The current `Editor` class has become a central place for too many responsibilities:

- window layout
- hierarchy rendering
- inspector rendering
- content browser rendering
- world load/save actions
- entity creation and deletion
- selection and picking
- simulation controls
- editor UI state buffers

This makes the class harder to read, test, extend, and maintain.

The goal of decomposition is to split the editor into smaller classes with clear ownership while keeping behavior stable during migration.

## Current Problems

1. `Editor` mixes UI drawing, world mutations, and transient UI state in one file.
2. A single change in one panel can easily affect unrelated editor code.
3. Selection, content browser, hierarchy, and inspector all share buffers through one class.
4. The file is already large enough that adding new features increases risk of regressions.
5. Reuse is difficult because most features are tightly coupled to `Editor.cpp`.

## Target Architecture

### 1. `EditorState`

Owns persistent editor-side state that is not a world component.

Suggested responsibility:

- selected entity
- simulation state
- window visibility flags
- world path
- assets path
- creation form values
- inspector temporary buffers
- content browser selection state

Why:

- UI state becomes explicit and centralized
- panels can work with shared state without storing everything in `Editor`

### 2. `EditorSelectionService`

Owns selection logic and scene picking.

Suggested responsibility:

- mouse picking
- selected entity updates
- validation of selected entity
- helper queries for current selection

Why:

- selection logic is separate from panel rendering
- easier to improve later with gizmos, outline logic, multi-select

### 3. `EditorWorldActions`

Owns world mutation commands used by the editor.

Suggested responsibility:

- load world
- save world
- clear world
- reload default world
- create empty
- create cube
- create model
- create lights
- delete selected entity

Why:

- editor commands become reusable and testable
- UI panels call actions instead of directly mutating systems

### 4. `WorldPanel`

Owns the `World` window UI.

Suggested responsibility:

- camera info
- world stats
- load/save controls
- simulation buttons
- drag-and-drop model spawn target

Why:

- separates top-level world controls from the rest of editor UI

### 5. `HierarchyPanel`

Owns hierarchy tree rendering and scene list actions.

Suggested responsibility:

- scene tree
- selection from hierarchy
- context menu actions
- quick add/delete buttons

Why:

- hierarchy becomes independent from inspector and content browser

### 6. `InspectorPanel`

Owns selected object editing UI.

Suggested responsibility:

- name
- visibility/enabled flags
- transform
- material editing
- physics editing
- light editing

Why:

- inspector usually grows the fastest
- isolating it early reduces future merge pain

### 7. `ContentBrowserPanel`

Owns assets tree and file interactions.

Suggested responsibility:

- assets root display/edit
- directory tree
- asset selection
- drag-and-drop payloads

Why:

- content browser has its own state and will likely grow previews and filters later

### 8. `EditorMenuBar`

Owns top menu rendering.

Suggested responsibility:

- file menu
- simulation menu
- window menu

Why:

- keeps top-level UI flow separate from per-window logic

## Recommended Folder Layout

Example:

```text
src/editor/
  Editor.h
  Editor.cpp
  EditorState.h
  EditorSelectionService.h
  EditorSelectionService.cpp
  EditorWorldActions.h
  EditorWorldActions.cpp
  panels/
    WorldPanel.h
    WorldPanel.cpp
    HierarchyPanel.h
    HierarchyPanel.cpp
    InspectorPanel.h
    InspectorPanel.cpp
    ContentBrowserPanel.h
    ContentBrowserPanel.cpp
    EditorMenuBar.h
    EditorMenuBar.cpp
```

## Migration Strategy

### Phase 1: State Extraction

Move buffers and flags from `Editor` into `EditorState`.

Expected result:

- behavior unchanged
- `Editor` becomes thinner
- easier to pass shared state to future panel classes

### Phase 2: Selection Extraction

Move:

- `TrySelectObject`
- selection validation
- highlight-related selection flow

into `EditorSelectionService`.

Expected result:

- scene picking becomes isolated
- future work like multi-select or viewport gizmos becomes easier

### Phase 3: World Actions Extraction

Move creation, deletion, save/load, and world reset commands into `EditorWorldActions`.

Expected result:

- panels no longer directly contain large world mutation branches
- action behavior becomes consistent across menu, hierarchy, and inspector

### Phase 4: Panel Split

Create dedicated panel classes one by one:

1. `EditorMenuBar`
2. `WorldPanel`
3. `HierarchyPanel`
4. `InspectorPanel`
5. `ContentBrowserPanel`

Expected result:

- `Editor.cpp` becomes composition-only
- each panel owns only one UI area

### Phase 5: Shared Utilities

Extract repeated helpers if needed:

- asset path helpers
- entity label helpers
- drag-and-drop helpers
- transform editing helpers

Expected result:

- less duplication between hierarchy, inspector, and content browser

## Final Role Of `Editor`

After decomposition, `Editor` should mostly do orchestration:

- own `EditorState`
- own panel objects
- own services/actions
- call `BeginMainMenuBar`/panel draw sequence
- connect editor systems with app systems

It should not contain most detailed UI or mutation logic anymore.

## Practical First Step

The safest first refactor is:

1. create `EditorState`
2. move all buffers and visibility flags into it
3. update `Editor` methods to use `m_state`

This gives immediate structure without changing editor behavior.

## Notes

- Keep migration incremental: one extraction at a time.
- Avoid changing functionality and architecture in the same step.
- Preserve existing editor behavior until the decomposition is complete.
- Add comments only at ownership boundaries, not on every trivial field.
