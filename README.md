# Trickster Rich Presence Client

## ✨ Features

- Displays player status (**Login Screen / In-Game**)
- Shows:
  - Character Name  
  - Character Job  
  - Character Level  
  - Current Map  
- Includes:
  - A **general area map image**
  - A **character job image**
- Designed primarily for **S2 game clients**

---

## ⚠ Compatibility Notes

- This project may require modifications to work with your own private server client (for example, memory pointers may differ).
- You must create your own **DLL injector** to inject this library into the game client.
  - The recommended approach is to use a **custom game launcher** for injection.

---

## 📝 Customization

For personal use, it is recommended to rename the project and files to something other than:

`PTRPC (Prifma Trickster Rich Presence Client)`

This helps avoid confusion and keeps your implementation unique.

---

## 🖥 Architecture

- **32-bit only** by default  
  - Can be adapted for **64-bit** with changes
- On my server:
  - The launcher is **32-bit** for retro-compatibility  
  - The DLL is also **32-bit**

---

## 📄 License & Usage

This project is provided **free of charge** and **as-is**.  
See the examples for intended use cases.

- ❌ No donations accepted  
- ❌ No resale permitted  
- ✅ Free for learning and personal server projects  
