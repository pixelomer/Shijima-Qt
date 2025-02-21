# Shijima-Qt API Documentation

Base URL: http://127.0.0.1:32456/shijima/api/v1

## GET /mascots

Returns a list of mascots that are on the screen.

**Sample response:**

```json
{
    "mascots": [
        {
            "active_behavior": "ClimbIEWall",
            "anchor": {
                "x": 67.2,
                "y": 225.63864462595598
            },
            "data_id": 0,
            "id": 35,
            "name": "Default Mascot"
        },
        {
            "active_behavior": "Fall",
            "anchor": {
                "x": 368,
                "y": 863
            },
            "data_id": 0,
            "id": 36,
            "name": "Default Mascot"
        }
    ]
}
```

## POST /mascots

Spawns a new mascot. One of `name` or `data_id` must be specified.

**Request format:**

```json
{
    "name": "string",
    "data_id": "int",
    "anchor": {
        "x": "double",
        "y": "double"
    },
    "behavior": "string"
}
```

**Sample request:**

```json
{
    "name": "Default Mascot",
    "anchor": {
        "x": 150,
        "y": 150
    },
    "behavior": "SplitIntoTwo"
}
```

**Sample response:**

```json
{
    "mascot": {
        "active_behavior": null,
        "anchor": {
            "x": 150,
            "y": 150
        },
        "data_id": 0,
        "id": 40,
        "name": "Default Mascot"
    }
}
```

## DELETE /mascots

Dismisses all mascots.

## GET /mascots/:id

Gets data for one mascot.

**Sample response:**

```json
{
    "mascot": {
        "active_behavior": "ClimbIEWall",
        "anchor": {
            "x": 67.2,
            "y": 225.63864462595598
        },
        "data_id": 0,
        "id": 35,
        "name": "Default Mascot"
    }
}
```

## PUT /mascots/:id

Alters the state of an existing mascot.

**Request format:**

```json
{
    "id": "int",
    "anchor": {
        "x": "double",
        "y": "double"
    },
    "behavior": "string"
}
```

**Sample request:**

```json
{
    "id": 4,
    "anchor": {
        "x": 150,
        "y": 150
    },
    "behavior": "SplitIntoTwo"
}
```

**Sample response:**

```json
{
    "mascot": {
        "active_behavior": "SitDown",
        "anchor": {
            "x": 150,
            "y": 150
        },
        "data_id": 79,
        "id": 4,
        "name": "Jenny"
    }
}
```

## GET /loadedMascots

Returns a list of mascots that are loaded into Shijima-Qt and can be spawned.

**Sample response:**

```json
{
    "loaded_mascots": [
        {
            "id": 0,
            "name": "Default Mascot"
        },
        {
            "id": 79,
            "name": "Jenny"
        },
        {
            "id": 78,
            "name": "niko"
        }
    ]
}
```

## GET /loadedMascots/:id

Returns information about a specific loaded mascot.

**Sample response:**

```json
{
    "loaded_mascot": {
        "id": 79,
        "name": "Jenny"
    }
}
```

## GET /loadedMascots/:id/preview.png

Returns the preview image for a loaded mascot.
