function showBootsplash() {
  const screen = document.getElementById('screen');
  screen.innerHTML = `
    <div class="bootsplash">
      <div class="limitless-logo">∞</div>
      <div class="edition">LimitlessOS Enterprise Edition</div>
      <div class="tagline">Fast. Secure. Limitless.</div>
    </div>
  `;
}

function showInstaller() {
  const screen = document.getElementById('screen');
  screen.innerHTML = `
    <div class="installer">
      <h2>LimitlessOS Installer</h2>
      <div class="options">
        <div class="option">Install Enterprise Edition</div>
        <div class="option">Install Developer Edition</div>
        <div class="option">Install Gaming Edition</div>
        <div class="option">Custom/Advanced Install</div>
      </div>
      <div class="features">
        <strong>Optional Features:</strong> Pentest Suite, Dev Tools, Gaming Stack, Virtualization, Accessibility, etc.
      </div>
      <div style="margin-top:24px; text-align:right;">
        <button style="background:#00e6d0;color:#222;padding:10px 32px;border:none;border-radius:8px;font-size:1em;cursor:pointer;">Continue</button>
      </div>
    </div>
  `;
}

function showDesktop() {
  const screen = document.getElementById('screen');
  screen.innerHTML = `
    <div class="desktop">
      <div class="wallpaper"></div>
      <div class="icons">
        <div class="icon" title="Home">🏠</div>
        <div class="icon" title="App Store">🛒</div>
        <div class="icon" title="Pentest Suite">🛡️</div>
        <div class="icon" title="Terminal">💻</div>
      </div>
      <div class="menu">
        <div><strong>Menu</strong></div>
        <div>Apps</div>
        <div>Settings</div>
        <div>Monitor</div>
      </div>
      <div class="taskbar">
        <div>Time: 12:34 | Network: Connected</div>
        <div>Notifications: 3</div>
      </div>
    </div>
  `;
}

// Show bootsplash by default
showBootsplash();
