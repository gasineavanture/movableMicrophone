#include "web_page.h"

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="sl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Kontrola mikrofona</title>

    <style>
        body {
            font-family: 'Segoe UI', sans-serif;
            background-color: #eef6ff;
            color: #1f2d3d;
            margin: 0;
            padding: 1rem;
            user-select: none;
        }

        h1 {
            text-align: center;
            margin-bottom: 1rem;
            color: #0f4c81;
        }

        h2 {
            color: #174f7f;
        }

        /* ===== GRID LAYOUT ===== */
        .main-container {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 1rem;
            max-width: 1000px;
            margin: 0 auto;
        }

        /* PANELI */
        .left-panel,
        .right-panel {
            display: flex;
            flex-direction: column;
            gap: 1rem;
        }

        /* KARTICE */
        .card {
            background-color: #ffffff;
            padding: 1rem;
            border-radius: 14px;
            border: 1px solid #d6e8f7;
            box-shadow: 0 6px 18px rgba(28, 93, 153, 0.12);
            text-align: center;
        }

        .person-form {
            display: grid;
            grid-template-columns: 1.5fr 1fr auto;
            gap: 0.5rem;
            margin-bottom: 1rem;
        }

        .person-form input {
            min-width: 0;
            padding: 0.7rem;
            border: 1px solid #c7dff3;
            border-radius: 8px;
            font-size: 1rem;
            background-color: #ffffff;
            color: #1f2d3d;
        }

        .person-form button {
            width: auto;
            margin-top: 0;
            padding: 0.7rem 1rem;
        }

        .save-card table {
            width: 100%;
            border-collapse: collapse;
        }

        .save-card th {
            padding: 0.6rem;
            color: #1f5f99;
            font-size: 0.85rem;
            text-align: left;
        }

        .save-card td {
            padding: 0.35rem;
            border-top: 1px solid rgba(31, 95, 153, 0.16);
            text-align: left;
        }

        .save-card td:nth-child(2), td:nth-child(3) {
            width: 14rem;
        }

        .save-card td:last-child {
            width: 14rem;
            text-align: right;
        }

        .person-actions {
            display: flex;
            justify-content: flex-end;
            gap: 0.35rem;
        }

        .person-row-active {
            background-color: #dceeff;
            font-weight: bold;
        }

        .speed-presets {
            display: flex;
            gap: 0.5rem;
            margin-top: 0.75rem;
        }

        .speed-presets button {
            flex: 1;
            margin-top: 0;
            padding: 0.7rem 0.4rem;
            font-size: 0.95rem;
        }

        .speed-presets button.selected {
            font-weight: bold;
            background-color: #1f5f99;
            box-shadow: 0 0 0 2px #1769c2 inset;
        }

        .save-card td input {
            box-sizing: border-box;
            width: 100%;
            padding: 0.55rem;
            border: 1px solid rgba(31, 95, 153, 0.30);
            border-radius: 7px;
            font-size: 1rem;
        }

        .save-card td button {
            width: auto;
            margin: 0;
            padding: 0.55rem 0.75rem;
            font-size: 0.9rem;
        }

        /* REFERENCA ČEZ OBA STOLPCA */
        .reference-card {
            grid-column: 1 / -1;
        }

        #positionValue {
            font-size: 1.5rem;
            font-weight: bold;
        }

        /* GUMBI */
        button {
            width: 100%;
            padding: 1rem;
            margin-top: 0.5rem;
            border: none;
            background-color: #2f80ed;
            color: white;
            font-size: 1.1rem;
            font-weight: 600;
            border-radius: 12px;
            cursor: pointer;
            touch-action: none;
            box-shadow: 0 3px 8px rgba(47, 128, 237, 0.20);
            transition: background-color 0.15s ease, transform 0.15s ease, box-shadow 0.15s ease;
        }

        button:hover {
            background-color: #256fce;
            box-shadow: 0 4px 10px rgba(47, 128, 237, 0.26);
        }

        button:active {
            background-color: #1f67c1;
        }

        /* SELECT */
        select {
            width: 100%;
            padding: 0.8rem;
            font-size: 1rem;
            border-radius: 10px;
            border: 1px solid #c7dff3;
            background-color: #ffffff;
            color: #1f2d3d;
        }

        /* SLIDER */
        .vertical-slider-container {
            height: 300px;
            display: flex;
            align-items: center;
            justify-content: center;
        }

        .vertical-slider {
            transform: rotate(-90deg);
            width: 260px;
        }

        /* PRIKAZ HITROSTI */
        #speedValue {
            margin-top: 1rem;
            font-size: 1.1rem;
            font-weight: bold;
        }

        /* ===== MOBILE ===== */
        @media (max-width: 700px) {
            .main-container {
                grid-template-columns: 1fr;
            }

            .reference-card {
                grid-column: 1;
            }

            .person-form {
                grid-template-columns: 1fr 1fr;
            }

            .person-form button {
                grid-column: 1 / -1;
            }
        }
    </style>
</head>
<body>

<h1>Kontrola mikrofona</h1>

<div class="main-container">

    <!-- LEVI -->
    <div class="left-panel">

        <div class="card save-card">
            <h2>Nastavitve višine oseb</h2>
                
            <div>
                <input type="checkbox" id="hideEmptyPeople" onchange="filterPeople()">
                <label for="hideEmptyPeople">Skrij nenastavljene osebe</label>
            </div>

            <table>
                <tbody id="peopleTableBody">
                </tbody>
            </table>
        </div>
    </div>

    <!-- DESNI -->
    <div class="right-panel">
        <div class="card manual-control">
            <h2>Ročno upravljanje</h2>

            <button
                ontouchstart="startMove('/move?dir=up', event)"
                ontouchend="stopMove('/stop', event)"
                onmousedown="startMove('/move?dir=up', event)"
                onmouseup="stopMove('/stop', event)">
                ⬆ Dvigni
            </button>

            <button
                ontouchstart="startMove('/move?dir=down', event)"
                ontouchend="stopMove('/stop', event)"
                onmousedown="startMove('/move?dir=down', event)"
                onmouseup="stopMove('/stop', event)">
                ⬇ Spusti
            </button>

            <div class="speed-presets">
                <button data-speed="15" onclick="setManualSpeed(15)">Počasi</button>
                <button data-speed="25" class="selected" onclick="setManualSpeed(25)">Normalno</button>
                <button data-speed="35" onclick="setManualSpeed(35)">Hitro</button>
            </div>
        </div>
    </div>

    <!-- REFERENCA -->
    <div class="card reference-card">
        <h2>Trenutna pozicija</h2>
        <div id="positionValue">Pridobivam...</div>
    </div>

    <div class="card reference-card">
        <h2>Referenčna točka</h2>
        <button onclick="goHome()">Pojdi na referenco</button>
    </div>

</div>

<script>
    let moveInterval = null;
    let moveSpeed = 150;
    let currentUrl = null;
    let psramJson = null;
    const encoderCountsPerCm = 10 * 425;
    let blocked = false;

    async function fetchJson(url) {
        try {
            const response = await fetch(url);
            const data = await response.json();
            if (!response.ok) throw new Error('HTTP ' + response.status);
            psramJson = data;
            renderPeopleTable(data);
        } catch (error) {
            console.error(error);
            return;
        }
    }

    async function updatePosition() {
        if (blocked) return;
        blocked = true;

        try {
            const response = await fetch('/position');
            const data = await response.json();
            if (!response.ok) throw new Error('HTTP ' + response.status);
            document.getElementById('positionValue').innerText =
                'Trenutno: ' + data.currentCm.toFixed(1) +
                ' cm | Cilj: ' + data.targetCm.toFixed(1) + ' cm';
            setActivePerson(data.activePersonId);
            setActiveSpeed(data.speed);
        } catch (error) {
            console.error(error);
            return;
        } finally {
            blocked = false;
        }
    }

    function renderPeopleTable(data) {
        const tbody = document.getElementById("peopleTableBody");
        tbody.innerHTML = "";
        const hideEmptyPeople = document.getElementById("hideEmptyPeople").checked;

        data.forEach((person, id) => {
            const heightCm = Number(person.height) / encoderCountsPerCm;
            const row = document.createElement("tr");
            row.dataset.personId = id;
            if (hideEmptyPeople && String(person.name).trim() === "") {
                row.style.display = "none";
            }
            row.innerHTML = `
                <td>${id +1}</td>
                <td>
                    <input
                        type="text"
                        value="${person.name}"
                        class="person-name"
                        onChange="updatePerson(${id}, this)"
                    >
                </td>
                <td>
                    <input
                        type="number"
                        value="${heightCm.toFixed(1)}"
                        class="person-height"
                        onChange="updatePerson(${id}, this)"
                    >
                </td>
                <td>
                    <div class="person-actions">
                        <button onclick="fillCurrentPosition(this)">Trenutno</button>
                        <button onclick="goInPosition(${id})">Pojdi v pozicijo</button>
                    </div>
                </td>
            `;
            tbody.appendChild(row);
        });
    }

    async function goInPosition(id) {
        try {
            const response = await fetch('/person?id=' + id);
            const data = await response.text();
            if (!response.ok) throw new Error('HTTP ' + response.status);
            console.log(data);
        } catch (error) {
            alert(error.message);
            return;
        }
        updatePosition();
    }

    function setActivePerson(id) {
        document.querySelectorAll('#peopleTableBody tr').forEach(row => {
            row.classList.toggle(
                'person-row-active',
                Number(row.dataset.personId) === Number(id)
            );
        });
    }

    function filterPeople() {
        if (psramJson) {
            renderPeopleTable(psramJson);
        }
    }

    async function fillCurrentPosition(button) {
        if (!confirm('Ali želite nastaviti trenutno višino mikrofona za to osebo?')) return;
        let id;
        try {
            const response = await fetch('/position');
            const position = await response.json();
            if (!response.ok) throw new Error('HTTP ' + response.status);
            const row = button.closest('tr');
            id = Number(row.dataset.personId);
            row.querySelector('.person-height').value = position.currentCm.toFixed(1);
        } catch (error) {
            alert(error.message);
            return;
        }
        updatePerson(id, button);
    }

    async function updatePerson(id, button) {
        const row = button.closest('tr');
        const newName = row.querySelector('.person-name').value.trim();
        const newHeightCm = Number(row.querySelector('.person-height').value);
        if (!psramJson || !Number.isFinite(newHeightCm) || newHeightCm < 0) {
            alert('Vnesi ime osebe in veljavno višino.');
            return;
        }
        try {
            const response = await fetch('/json/patch', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ id, name: newName, height: Math.round(newHeightCm * encoderCountsPerCm) })
            });
            const updatedData = await response.json();
            if (!response.ok) throw new Error('HTTP ' + response.status);
            psramJson = updatedData;
            renderPeopleTable(updatedData);
        } catch (error) {
            alert(error.message);
        }
    }

    async function refreshPeople() {
        if (blocked) return;
        blocked = true;

        try {
            const response = await fetch('/json/read');
            const data = await response.json();
            if (!response.ok) throw new Error('HTTP ' + response.status);
            if (JSON.stringify(data) !== JSON.stringify(psramJson)) {
                psramJson = data;
                renderPeopleTable(data);
            }
        } catch (error) {
            console.error(error);
            return;
        } finally {
            blocked = false;
        }
    }

    async function startMove(url, event) {
        event.preventDefault();
        currentUrl = url;
        clearInterval(moveInterval);
        moveInterval = setInterval(async () => {
            try {
                const response = await fetch(url);
                await response.text();
                if (!response.ok) throw new Error('HTTP ' + response.status);
            } catch (error) {
                console.error(error);
            }
        }, moveSpeed);
            try {
                const response = await fetch(url);
                await response.text();
                if (!response.ok) throw new Error('HTTP ' + response.status);
            } catch (error) {
                console.error(error);
            }
    }

    async function stopMove(url, event) {
        event.preventDefault();
        clearInterval(moveInterval);
        moveInterval = null;
        currentUrl = null;
        try {
            const response = await fetch(url);
            const data = await response.text();
            if (!response.ok) throw new Error('HTTP ' + response.status);

        } catch (error) {
            console.error(error);
            return;
        }
    }

    async function setManualSpeed(speed, button) {
        try {
            const response = await fetch('/setSpeed?speed=' + speed);
            const data = await response.text();
            if (!response.ok) throw new Error('HTTP ' + response.status);
            setActiveSpeed(speed);
        } catch (error) {
            console.error(error);
            return;
        }
    }

    function setActiveSpeed(speed) {
        document.querySelectorAll('.speed-presets button').forEach(preset => {
            preset.classList.toggle(
                'selected',
                Number(preset.dataset.speed) === Number(speed)
            );
        });
    }

    async function goHome() {
        try {
            const response = await fetch('/calibrate');
            const data = await response.text();
            if (!response.ok) throw new Error('HTTP ' + response.status);

        } catch (error) {
            console.error(error);
            return;
        }
    }

    window.onload = function() {
        fetchJson('/json/read');
        updatePosition();
        setInterval(updatePosition, 501);
        setInterval(refreshPeople, 1000);
    };
</script>

</body>
</html>
)rawliteral";
