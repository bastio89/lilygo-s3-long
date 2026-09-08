import { writeFileSync } from 'fs';

const FONT = '<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Montserrat:wght@400;600&display=swap">';

const CSS = `
body{margin:0;background:#0B0D10;font-family:Montserrat,system-ui,-apple-system,sans-serif;-webkit-font-smoothing:antialiased}
.dev,.sys{--bg:#0E1116;--panel:#191F27;--raise:#242C37;--line:#2E3743;
 --tx:#F2F5F8;--tx2:#9AA6B4;--tx3:#6B7684;--txdis:#48515D;
 --acc:#4FA3FF;--accs:#8CC5FF;--good:#5BD68A;--warn:#FFA23D;--bad:#FF5A52;
 --stopbg:#2A1A1E;--dotoff:#3A4553;--thumb:#465262}
.dev{position:relative;width:640px;height:180px;overflow:hidden;box-sizing:border-box;
 background:var(--bg);color:var(--tx)}
.dev.nacht{--bg:#070A0D;--panel:#10141A;--raise:#161C24;--line:#1E2630;
 --tx:#C9D3DE;--tx2:#7C8794;--tx3:#59636F;--txdis:#333C46;
 --acc:#3D82CC;--accs:#5AA0E6;--good:#3E9463;--warn:#C97F30;--bad:#A33B34;
 --stopbg:#1A1216;--dotoff:#232B35;--thumb:#2A323C}

.bar{position:absolute;left:0;top:0;width:640px;height:28px}
.bar .hgt{position:absolute;left:12px;top:0;height:28px;line-height:28px;font-size:16px;font-weight:600}
.bar .hgt i{font-style:normal;font-size:12px;font-weight:400;color:var(--tx2);margin-left:3px}
.bar .sep{position:absolute;left:120px;top:8px;width:1px;height:12px;background:var(--line)}
.bar .pg{position:absolute;left:132px;top:0;height:28px;line-height:28px;font-size:14px;
 color:var(--tx2);max-width:145px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.bar .dots{position:absolute;left:287px;top:12px;display:flex;gap:5px;align-items:center}
.bar .dots b{width:4px;height:4px;border-radius:2px;background:var(--dotoff);display:block}
.bar .dots b.on{width:12px;background:var(--acc)}
.bar .dat{position:absolute;left:452px;top:0;width:84px;text-align:right;height:28px;line-height:28px;font-size:12px;color:var(--tx3)}
.bar .clk{position:absolute;left:544px;top:0;width:56px;text-align:right;height:28px;line-height:28px;font-size:16px;font-weight:600}
.bar .wifi{position:absolute;left:612px;top:9px}
.bar .rule{position:absolute;left:0;top:27px;width:640px;height:1px;background:var(--line)}

.area{position:absolute;left:0;top:28px;width:640px;height:152px}
.tile{position:absolute;background:var(--panel);border-radius:8px;box-sizing:border-box}
.focus{left:8px;top:8px;width:244px;height:136px}
.band{position:absolute;left:0;top:0;bottom:0;width:4px;border-radius:8px 0 0 8px;background:var(--tx3)}
.band.acc{background:var(--acc)}.band.warn{background:var(--warn)}
.band.bad{background:var(--bad)}.band.good{background:var(--good)}
.eyebrow{position:absolute;left:20px;top:14px;font-size:12px;color:var(--tx3)}
.val{position:absolute;left:20px;top:34px;font-size:48px;font-weight:600;line-height:1;letter-spacing:-1px;white-space:nowrap}
.val i{font-style:normal;font-size:20px;font-weight:400;color:var(--tx2);margin-left:6px;letter-spacing:0}
.val.dim{color:var(--tx3)}
.val.warn{color:var(--warn)}
.val.bad{color:var(--bad)}
.sub{position:absolute;left:20px;top:98px;font-size:16px;color:var(--tx2);white-space:nowrap;
 max-width:206px;overflow:hidden;text-overflow:ellipsis}
.sub.acc{color:var(--acc)}.sub.warn{color:var(--warn)}.sub.bad{color:var(--bad)}
.mid{position:absolute;left:20px;top:44px;font-size:24px;font-weight:600;line-height:1.15;width:206px;
 overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.mid2{position:absolute;left:20px;top:74px;font-size:16px;color:var(--tx2);width:206px;
 overflow:hidden;text-overflow:ellipsis;white-space:nowrap}

.btn{position:absolute;background:var(--panel);border-radius:8px;display:flex;flex-direction:column;
 align-items:center;justify-content:center;gap:1px;font-size:20px;font-weight:600;color:var(--tx)}
.btn small{font-size:12px;font-weight:400;color:var(--tx2)}
.btn.on{background:var(--acc);color:#0E1116}
.btn.on small{color:#0E1116;opacity:.65}
.btn.hint{box-shadow:inset 0 0 0 2px var(--acc)}
.btn.dis{color:var(--tx3)}.btn.dis small{color:var(--txdis)}
.up{left:260px;top:8px;width:96px;height:64px}
.dn{left:260px;top:80px;width:96px;height:64px}
.tall{left:260px;top:8px;width:96px;height:136px}
.p1{left:364px;top:8px;width:108px;height:64px}
.p2{left:480px;top:8px;width:108px;height:64px}
.p3{left:364px;top:80px;width:108px;height:64px}
.p4{left:480px;top:80px;width:108px;height:64px}
.stop{position:absolute;left:596px;top:8px;width:36px;height:136px;border-radius:8px;
 background:var(--stopbg);display:flex;align-items:center;justify-content:center}
.stop.hot{background:var(--bad)}

.prog{position:absolute;left:20px;height:4px;border-radius:2px;background:var(--line)}
.prog span{display:block;height:4px;border-radius:2px;background:var(--acc)}

.stat{position:absolute;background:var(--panel);border-radius:8px;box-sizing:border-box;padding:8px 12px}
.stat u{display:block;text-decoration:none;font-size:12px;color:var(--tx3)}
.stat b{display:block;font-size:20px;font-weight:600;margin-top:4px}

.row{position:absolute;left:8px;width:624px;height:44px;background:var(--panel);border-radius:8px}
.row .lb{position:absolute;left:16px;top:0;height:44px;line-height:44px;font-size:16px;color:var(--tx2)}
.row .rt{position:absolute;right:14px;top:0;height:44px;display:flex;align-items:center;gap:8px}
.pill{background:var(--raise);border-radius:6px;height:28px;display:flex;align-items:center;
 padding:0 10px;gap:8px;font-size:14px;color:var(--tx)}
.step{background:var(--raise);border-radius:6px;width:30px;height:28px;display:flex;
 align-items:center;justify-content:center;font-size:20px;font-weight:600;color:var(--tx)}
.num{font-size:16px;font-weight:600;min-width:46px;text-align:center}
.sl{position:relative;width:200px;height:28px;display:flex;align-items:center}
.sl .tr{position:absolute;left:0;right:0;height:4px;border-radius:2px;background:var(--line)}
.sl .fl{position:absolute;left:0;height:4px;border-radius:2px;background:var(--acc)}
.sl .kn{position:absolute;width:18px;height:18px;border-radius:9px;background:var(--acc)}
.scroll{position:absolute;left:634px;top:8px;width:2px;height:136px;border-radius:1px;background:var(--line)}
.scroll span{display:block;width:2px;border-radius:1px;background:var(--thumb)}

.lrow{position:absolute;background:var(--panel);border-radius:8px;box-sizing:border-box}
.lrow .t{position:absolute;left:14px;top:0;bottom:0;display:flex;align-items:center;
 font-size:16px;font-weight:600;color:var(--tx)}
.lrow .n{position:absolute;left:66px;top:0;bottom:0;display:flex;align-items:center;
 font-size:16px;color:var(--tx2);max-width:200px;overflow:hidden;white-space:nowrap;text-overflow:ellipsis}
.lrow .d{position:absolute;right:14px;top:0;bottom:0;display:flex;align-items:center;
 font-size:12px;color:var(--tx3)}

.card{position:absolute;background:var(--panel);border-radius:8px;box-sizing:border-box;padding:10px 12px}
.card .cb{position:absolute;left:0;top:0;bottom:0;width:4px;border-radius:8px 0 0 8px;background:transparent}
.card.act .cb{background:var(--acc)}
.card n{display:block;font-size:16px;font-weight:600;color:var(--tx);font-style:normal}
.card v{display:block;font-size:12px;color:var(--tx3);margin-top:5px;font-style:normal;
 white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.card.free{background:transparent;box-shadow:inset 0 0 0 1px var(--line)}
.card.free n{color:var(--tx3);font-weight:400}
`;

const wifi = (c = 'var(--tx2)') =>
  '<svg class="wifi" width="16" height="12" viewBox="0 0 16 12" fill="none">' +
  '<path d="M1 3.6C3 1.6 5.4 0.6 8 0.6s5 1 7 3" stroke="' + c + '" stroke-width="1.5" stroke-linecap="round"/>' +
  '<path d="M3.6 6.4C4.9 5.1 6.4 4.5 8 4.5s3.1 0.6 4.4 1.9" stroke="' + c + '" stroke-width="1.5" stroke-linecap="round"/>' +
  '<circle cx="8" cy="9.8" r="1.5" fill="' + c + '"/></svg>';

const wifiOff = () =>
  '<svg class="wifi" width="16" height="12" viewBox="0 0 16 12" fill="none">' +
  '<path d="M1 3.6C3 1.6 5.4 0.6 8 0.6s5 1 7 3" stroke="#4A535F" stroke-width="1.5" stroke-linecap="round"/>' +
  '<circle cx="8" cy="9.8" r="1.5" fill="#4A535F"/>' +
  '<path d="M2 11 L14 1" stroke="var(--warn)" stroke-width="1.5" stroke-linecap="round"/></svg>';

const arrow = (dir, c = 'currentColor') =>
  dir === 'up'
    ? '<svg width="30" height="18" viewBox="0 0 30 18"><path d="M15 1 L29 17 L1 17 Z" fill="' + c + '"/></svg>'
    : '<svg width="30" height="18" viewBox="0 0 30 18"><path d="M15 17 L1 1 L29 1 Z" fill="' + c + '"/></svg>';

const stopSq = (c = 'var(--bad)') =>
  '<svg width="16" height="16" viewBox="0 0 16 16"><rect x="0" y="0" width="16" height="16" rx="2" fill="' + c + '"/></svg>';

const play = (c = 'currentColor') =>
  '<svg width="24" height="26" viewBox="0 0 24 26"><path d="M3 2 L22 13 L3 24 Z" fill="' + c + '"/></svg>';
const pause = (c = 'currentColor') =>
  '<svg width="22" height="26" viewBox="0 0 22 26"><rect x="2" y="2" width="6" height="22" rx="1" fill="' + c + '"/>' +
  '<rect x="14" y="2" width="6" height="22" rx="1" fill="' + c + '"/></svg>';
const next = (c = 'currentColor') =>
  '<svg width="22" height="18" viewBox="0 0 22 18"><path d="M2 1 L14 9 L2 17 Z" fill="' + c + '"/>' +
  '<rect x="17" y="1" width="4" height="16" rx="1" fill="' + c + '"/></svg>';
const prev = (c = 'currentColor') =>
  '<svg width="22" height="18" viewBox="0 0 22 18"><path d="M20 1 L8 9 L20 17 Z" fill="' + c + '"/>' +
  '<rect x="1" y="1" width="4" height="16" rx="1" fill="' + c + '"/></svg>';
const chev = (c = 'var(--tx2)') =>
  '<svg width="10" height="6" viewBox="0 0 10 6"><path d="M1 1 L5 5 L9 1" stroke="' + c + '" stroke-width="1.6" fill="none" stroke-linecap="round"/></svg>';

const dots = (active, n = 7) => {
  if (active < 0) return '<div class="dots"></div>';
  let out = '';
  for (let i = 0; i < n; i++) out += '<b class="' + (i === active ? 'on' : '') + '"></b>';
  return '<div class="dots">' + out + '</div>';
};

function bar({ hgt = '73,5', page = 'Schreibtisch', active = 0, clock = '13:45',
               date = 'Mo 8. Sep', net = 'on', unit = 'cm' }) {
  const h = hgt === null
    ? '<div class="hgt" style="color:var(--tx3)">– – –</div>'
    : '<div class="hgt">' + hgt + (unit ? '<i>' + unit + '</i>' : '') + '</div>';
  return '<div class="bar">' + h +
    '<div class="sep"></div><div class="pg">' + page + '</div>' + dots(active) +
    '<div class="dat">' + date + '</div><div class="clk">' + clock + '</div>' +
    (net === 'on' ? wifi() : wifiOff()) + '<div class="rule"></div></div>';
}

function page({ file, title, body, barCfg = {}, nacht = false, extra = '' }) {
  const html = '<!doctype html>\n<html lang="de">\n<head>\n<meta charset="utf-8">\n' +
    '<script src="./support.js"><\/script>\n</head>\n<body>\n<x-dc>\n<helmet>\n' + FONT +
    '\n<style>' + CSS + extra + '</style>\n</helmet>\n' +
    '<div class="dev' + (nacht ? ' nacht' : '') + '">\n' + bar(barCfg) +
    '\n<div class="area">\n' + body + '\n</div>\n</div>\n</x-dc>\n</body>\n</html>\n';
  writeFileSync('design/' + file, html);
  return { file, title };
}

/* ---------------------------------------------------------------- Seiten --- */

const stopBtn = (hot = false) =>
  '<div class="stop' + (hot ? ' hot' : '') + '">' + stopSq(hot ? 'var(--bg)' : 'var(--bad)') + '</div>';

const upDn = (state = '') =>
  '<div class="btn up' + (state === 'up' ? ' on' : state === 'hintup' ? ' hint' : '') + '">' +
    arrow('up', state === 'up' ? '#0E1116' : 'currentColor') + '</div>' +
  '<div class="btn dn' + (state === 'dn' ? ' on' : '') + '">' + arrow('down') + '</div>';

const presets = (dis = false) => {
  const d = dis ? ' dis' : '';
  return '<div class="btn p1' + d + '">Sitzen<small>73,0</small></div>' +
    '<div class="btn p2' + d + '">Stehen<small>112,0</small></div>' +
    '<div class="btn p3">Platz 3<small>Box</small></div>' +
    '<div class="btn p4">Platz 4<small>Box</small></div>';
};

const boards = [];

/* 1 — Schreibtisch, Ruhezustand (Main) */
boards.push(page({
  file: 'Main.dc.html', title: 'Schreibtisch — bereit',
  barCfg: { page: 'Schreibtisch', active: 0 },
  body:
    '<div class="tile focus"><div class="band"></div>' +
    '<div class="val">73,5<i>cm</i></div><div class="sub">bereit</div></div>' +
    upDn() + presets() + stopBtn()
}));

/* 2 — Schreibtisch, Fahrt mit Ziel */
boards.push(page({
  file: 'SchreibtischFahrt.dc.html', title: 'Schreibtisch — Fahrt',
  barCfg: { page: 'Schreibtisch', active: 0, hgt: '89,2' },
  body:
    '<div class="tile focus"><div class="band acc"></div>' +
    '<div class="val">89,2<i>cm</i></div>' +
    '<div class="prog" style="top:88px;width:206px"><span style="width:88px"></span></div>' +
    '<div class="sub acc" style="top:104px">Ziel 112,0 cm</div></div>' +
    upDn('hintup') + presets() + stopBtn(true)
}));

/* 3 — Schreibtisch ohne Rückmeldung */
boards.push(page({
  file: 'SchreibtischOhneRueckmeldung.dc.html', title: 'Schreibtisch — keine Rückmeldung',
  barCfg: { page: 'Schreibtisch', active: 0, hgt: null },
  body:
    '<div class="tile focus"><div class="band warn"></div>' +
    '<div class="val dim">– – –<i>cm</i></div>' +
    '<div class="sub warn">Steuerbox meldet nichts</div></div>' +
    upDn() + presets(true) + stopBtn()
}));

/* 4 — Schreibtisch, Fehlercode */
boards.push(page({
  file: 'SchreibtischFehlercode.dc.html', title: 'Schreibtisch — Fehlercode',
  barCfg: { page: 'Schreibtisch', active: 0, hgt: 'ASr', unit: '' },
  body:
    '<div class="tile focus"><div class="band bad"></div>' +
    '<div class="val bad">ASr</div>' +
    '<div class="sub warn">Referenzfahrt nötig</div></div>' +
    upDn() + presets(true) + stopBtn()
}));

/* 5 — Wetter */
const statTile = (x, y, label, value) =>
  '<div class="stat" style="left:' + x + 'px;top:' + y + 'px;width:118px;height:64px">' +
  '<u>' + label + '</u><b>' + value + '</b></div>';

boards.push(page({
  file: 'Wetter.dc.html', title: 'Wetter',
  barCfg: { page: 'Wetter — Berlin', active: 1 },
  body:
    '<div class="tile focus"><div class="band"></div>' +
    '<div class="eyebrow">Berlin</div>' +
    '<div class="val">18<i>°C</i></div><div class="sub">Teils bewölkt</div></div>' +
    statTile(260, 8, 'min', '12°') + statTile(387, 8, 'max', '21°') + statTile(514, 8, 'gefühlt', '17°') +
    statTile(260, 80, 'Luftfeuchte', '62 %') + statTile(387, 80, 'Wind', '11 km/h') +
    statTile(514, 80, 'Regen', '20 %')
}));

/* 6 — Wetter ohne Netz */
boards.push(page({
  file: 'WetterOhneNetz.dc.html', title: 'Wetter — kein Netz',
  barCfg: { page: 'Wetter — Berlin', active: 1, net: 'off', clock: '13:45' },
  body:
    '<div class="tile focus"><div class="band warn"></div>' +
    '<div class="eyebrow">Berlin</div>' +
    '<div class="val dim">– – –<i>°C</i></div><div class="sub warn">Kein Netz</div></div>' +
    '<div class="tile" style="left:260px;top:8px;width:372px;height:136px">' +
    '<div style="position:absolute;left:22px;top:38px;font-size:16px;color:var(--tx2)">' +
    'Letzter Abruf 09:12 Uhr</div>' +
    '<div style="position:absolute;left:22px;top:64px;font-size:12px;color:var(--tx3)">' +
    'Nächster Versuch in 40 s</div>' +
    '<div class="prog" style="left:22px;top:96px;width:328px"><span style="width:104px"></span></div>' +
    '</div>'
}));

/* 7 — Einstellungen */
boards.push(page({
  file: 'Einstellungen.dc.html', title: 'Einstellungen',
  barCfg: { page: 'Einstellungen', active: 6 },
  body:
    '<div class="row" style="top:8px"><div class="lb">Helligkeit</div>' +
    '<div class="rt"><div class="sl"><div class="tr"></div><div class="fl" style="width:139px"></div>' +
    '<div class="kn" style="left:130px"></div></div><div class="num">180</div></div></div>' +
    '<div class="row" style="top:60px"><div class="lb">Display-Ruhe nach</div>' +
    '<div class="rt"><div class="pill" style="width:150px;justify-content:space-between">' +
    '<span>2 Minuten</span>' + chev() + '</div></div></div>' +
    '<div class="row" style="top:112px;height:32px;overflow:hidden">' +
    '<div class="lb" style="height:32px;line-height:32px">Fahrbereich min / max</div>' +
    '<div class="rt" style="height:32px"><div class="step">−</div><div class="num">60,0</div>' +
    '<div class="step">+</div><div class="step">−</div><div class="num">125,0</div>' +
    '<div class="step">+</div></div></div>' +
    '<div class="scroll"><span style="height:56px"></span></div>'
}));

/* 8 — Spotify */
boards.push(page({
  file: 'Spotify.dc.html', title: 'Spotify',
  barCfg: { page: 'Spotify', active: 3 },
  body:
    '<div class="tile focus"><div class="band good"></div>' +
    '<div class="eyebrow">Wohnzimmer</div>' +
    '<div class="mid">Everywhere</div><div class="mid2">Fleetwood Mac</div>' +
    '<div class="prog" style="top:108px;width:206px"><span style="width:76px"></span></div>' +
    '<div style="position:absolute;left:20px;top:118px;font-size:12px;color:var(--tx3)">1:42</div>' +
    '<div style="position:absolute;right:18px;top:118px;font-size:12px;color:var(--tx3)">4:14</div>' +
    '</div>' +
    '<div class="btn tall">' + pause() + '</div>' +
    '<div class="btn p1">' + prev() + '</div>' +
    '<div class="btn p2">' + next() + '</div>' +
    '<div class="btn p3">Leiser<small>−</small></div>' +
    '<div class="btn p4">Lauter<small>+</small></div>' +
    '<div class="stop" style="background:var(--panel)">' +
    '<div style="font-size:12px;color:var(--tx2);writing-mode:vertical-rl;' +
    'transform:rotate(180deg);letter-spacing:1px">STUMM</div></div>'
}));

/* 9 — Kalender */
const lrow = (y, t, n, d) =>
  '<div class="lrow" style="left:260px;top:' + y + 'px;width:372px;height:42px">' +
  '<div class="t">' + t + '</div><div class="n">' + n + '</div><div class="d">' + d + '</div></div>';

boards.push(page({
  file: 'Kalender.dc.html', title: 'Kalender',
  barCfg: { page: 'Kalender', active: 2 },
  body:
    '<div class="tile focus"><div class="band acc"></div>' +
    '<div class="eyebrow">in 12 Minuten</div>' +
    '<div class="val">09:30</div><div class="sub">Daily Standup</div></div>' +
    lrow(8, '11:00', 'Review Steuerbox', '45 min') +
    lrow(55, '13:30', 'Mittag mit Anna', '1 h') +
    lrow(102, '16:00', 'Retro', '30 min')
}));

/* 10 — Seitenraster */
const card = (x, y, name, value, act = false, free = false) =>
  '<div class="card' + (act ? ' act' : '') + (free ? ' free' : '') +
  '" style="left:' + x + 'px;top:' + y + 'px;width:150px;height:64px">' +
  '<div class="cb"></div><n>' + name + '</n>' + (value ? '<v>' + value + '</v>' : '') + '</div>';

boards.push(page({
  file: 'Seitenraster.dc.html', title: 'Seitenraster',
  barCfg: { page: 'Seitenraster', active: -1 },
  body:
    card(8, 8, 'Schreibtisch', '73,5 cm — bereit', true) +
    card(166, 8, 'Wetter', '18°, teils bewölkt') +
    card(324, 8, 'Kalender', '09:30 Daily Standup') +
    card(482, 8, 'Spotify', 'Go Your Own Way') +
    card(8, 80, 'Einstellungen', 'Helligkeit 180') +
    card(166, 80, 'Raumklima', '21,4° · 44 %') +
    card(324, 80, 'Abfahrten', 'M10 in 4 min') +
    card(482, 80, 'frei', '', false, true)
}));

/* 11 — Nachtvariante */
boards.push(page({
  file: 'Nacht.dc.html', title: 'Nachtvariante',
  nacht: true,
  barCfg: { page: 'Schreibtisch', active: 0, clock: '23:40', date: 'Mo 8. Sep' },
  body:
    '<div class="tile focus"><div class="band"></div>' +
    '<div class="val">73,5<i>cm</i></div><div class="sub">bereit</div></div>' +
    upDn() + presets() + stopBtn()
}));

/* 12 — Statuszeile, aufgerissen */
const cal = (x, y, w, txt, c = '#4FA3FF') =>
  '<div style="position:absolute;left:' + x + 'px;top:' + y + 'px;width:' + w + 'px;height:16px;' +
  'border-left:1px solid ' + c + ';border-right:1px solid ' + c + ';border-top:1px solid ' + c + ';' +
  'box-sizing:border-box"></div>' +
  '<div style="position:absolute;left:' + x + 'px;top:' + (y + 18) + 'px;width:' + w + 'px;' +
  'text-align:center;font-size:11px;color:' + c + '">' + txt + '</div>';

boards.push(page({
  file: 'Statuszeile.dc.html', title: 'Statuszeile',
  barCfg: { page: 'Wetter — Berlin', active: 1 },
  extra: '.dev{height:180px}',
  body:
    cal(4, 34, 122, 'tippen: Schreibtisch', '#4FA3FF') +
    cal(130, 34, 228, 'tippen: Seitenraster', '#4FA3FF') +
    cal(448, 34, 188, 'Uhr · Datum · WLAN', '#6B7684') +
    '<div style="position:absolute;left:8px;top:82px;width:624px;height:62px;background:var(--panel);' +
    'border-radius:8px;box-sizing:border-box;padding:12px 16px">' +
    '<div style="font-size:14px;color:var(--tx);font-weight:600">Zwei Tipper zu jeder Seite</div>' +
    '<div style="font-size:12px;color:var(--tx2);margin-top:6px;line-height:1.5">' +
    'Die Höhe steht immer oben — der häufigste Blick braucht keine Seite.</div></div>'
}));

/* ------------------------------------------------------------ System --- */

const SYSCSS = `
body{margin:0;background:#0B0D10;font-family:Montserrat,system-ui,sans-serif;-webkit-font-smoothing:antialiased}
.sys{width:1480px;box-sizing:border-box;padding:56px;background:#0B0D10;color:#F2F5F8}
.sys h1{font-size:34px;font-weight:600;margin:0;letter-spacing:-.5px}
.sys .lede{font-size:16px;color:#9AA6B4;margin-top:10px;max-width:820px;line-height:1.55}
.sec{margin-top:44px}
.sec h2{font-size:13px;font-weight:600;color:#6B7684;letter-spacing:1.4px;margin:0 0 18px;
 text-transform:uppercase}
.cols{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:48px}
.sw{display:flex;align-items:center;gap:14px;padding:7px 0}
.sw i{width:34px;height:34px;border-radius:6px;display:block;flex:none;
 box-shadow:inset 0 0 0 1px rgba(255,255,255,.08)}
.sw b{font-size:14px;font-weight:600;width:112px;flex:none}
.sw code{font-size:13px;color:#9AA6B4;font-family:ui-monospace,SFMono-Regular,Menlo,monospace;
 width:76px;flex:none}
.sw span{font-size:13px;color:#6B7684}
.ty{display:flex;align-items:baseline;gap:16px;padding:9px 0;border-bottom:1px solid #1B222B}
.ty em{font-style:normal;font-size:12px;color:#6B7684;width:96px;flex:none}
.ty s{text-decoration:none;color:#F2F5F8}
.rule-list{font-size:14px;color:#9AA6B4;line-height:1.75;margin:0;padding-left:20px}
.rule-list li{margin-bottom:9px}
.rule-list b{color:#F2F5F8;font-weight:600}
.parts{display:flex;flex-wrap:wrap;gap:26px}
.part{background:#12161C;border-radius:10px;padding:18px}
.part cap{display:block;font-size:12px;color:#6B7684;margin-top:12px;font-style:normal}
.icons{display:flex;gap:38px;flex-wrap:wrap}
.ic{text-align:center;width:96px}
.ic div{height:44px;display:flex;align-items:center;justify-content:center}
.ic span{font-size:12px;color:#9AA6B4}
.grid-demo{position:relative;width:640px;height:180px;background:#0E1116;border-radius:4px}
.gz{position:absolute;box-sizing:border-box;border:1px dashed #3E4B5A;border-radius:6px;
 font-size:11px;color:#6B7684;display:flex;align-items:center;justify-content:center;text-align:center}
.note{font-size:13px;color:#6B7684;margin-top:14px;line-height:1.6;max-width:560px}
`;

const swatch = (hex, name, use) =>
  '<div class="sw"><i style="background:' + hex + '"></i><b>' + name + '</b><code>' + hex +
  '</code><span>' + use + '</span></div>';

const ty = (label, style, sample) =>
  '<div class="ty"><em>' + label + '</em><s style="' + style + '">' + sample + '</s></div>';

const icon = (svg, name) => '<div class="ic"><div>' + svg + '</div><span>' + name + '</span></div>';

const sysBody =
  '<h1>SmartDesk — Designsystem</h1>' +
  '<div class="lede">Ein Baukasten für einen 640 × 180 Pixel breiten Streifen an der ' +
  'Tischkante. Alles ordnet sich nebeneinander: links der eine Wert, der die Seite ausmacht, ' +
  'rechts die Aktionen. Farbtoken und Schriftstufen sind die, die bereits im Flash liegen.</div>' +

  '<div class="sec"><div class="cols">' +
  '<div><h2>Farbtoken</h2>' +
  swatch('#0E1116', 'Grund', 'Hintergrund') +
  swatch('#191F27', 'Kachel', 'Flächen, Tasten') +
  swatch('#242C37', 'Kachel hoch', 'Auswahlfeld, Stepper') +
  swatch('#2E3743', 'Linie', 'Trenner, leere Balken') +
  swatch('#F2F5F8', 'Text', 'Werte, Tastenbeschriftung') +
  swatch('#9AA6B4', 'Text 2', 'Status, Labels') +
  swatch('#6B7684', 'Text 3', 'Einheiten, Diagnose') +
  swatch('#4FA3FF', 'Akzent', 'bedienbar, in Bewegung') +
  swatch('#8CC5FF', 'Akzent hell', 'gedrückt') +
  swatch('#5BD68A', 'Gut', 'erreicht, aktiv') +
  swatch('#FFA23D', 'Warnung', 'keine Daten, blockiert') +
  swatch('#FF5A52', 'Fehler', 'Not-Stopp, Fehlercode') +
  swatch('#070A0D', 'Nacht-Grund', 'gedimmte Variante') +
  '<div class="note">In RGB565 überleben flächige Farben; große weiche Verläufe nicht. ' +
  'Deshalb gibt es keinen einzigen Verlauf im System.</div></div>' +

  '<div><h2>Schriftskala — Montserrat</h2>' +
  ty('48 / SemiBold', 'font-size:48px;font-weight:600;letter-spacing:-1px;line-height:52px', '73,5') +
  ty('24 / SemiBold', 'font-size:24px;font-weight:600', 'Medientitel') +
  ty('20 / SemiBold', 'font-size:20px;font-weight:600', 'Taste') +
  ty('16 / SemiBold', 'font-size:16px;font-weight:600', 'Uhrzeit, Wert') +
  ty('16 / Regular', 'font-size:16px', 'Statustext, Listenzeile') +
  ty('14 / Regular', 'font-size:14px', 'Statuszeile, Seitenname') +
  ty('12 / Regular', 'font-size:12px', 'Einheit, Diagnose, Label') +
  '<div class="note">Sieben Zeichensätze: Regular in 12/14/16, SemiBold in 16/20/24/48. ' +
  'Keine Stufe, die nicht auf mindestens einem Artboard vorkommt.<br><br>' +
  'Über ASCII hinaus muss der Zeichensatz tragen: die Umlaute und ß, das Gradzeichen °, ' +
  'den Geviertstrich — für Seitennamen, den Halbgeviertstrich – für den Platzhalter ' +
  '„– – –“, das Minuszeichen − der Stepper und den Mittelpunkt · als Trenner. ' +
  'Acht Zeichen, die man beim Erzeugen leicht vergisst.</div></div>' +

  '<div><h2>Raster und Seitenrezept</h2>' +
  '<div class="grid-demo">' +
  '<div class="gz" style="left:0;top:0;width:640px;height:28px">Statuszeile 28 px — immer sichtbar</div>' +
  '<div class="gz" style="left:8px;top:36px;width:244px;height:136px">Fokuszone<br>244 × 136</div>' +
  '<div class="gz" style="left:260px;top:36px;width:96px;height:136px">Halten<br>96</div>' +
  '<div class="gz" style="left:364px;top:36px;width:108px;height:64px">108 × 64</div>' +
  '<div class="gz" style="left:480px;top:36px;width:108px;height:64px">108 × 64</div>' +
  '<div class="gz" style="left:364px;top:108px;width:108px;height:64px">108 × 64</div>' +
  '<div class="gz" style="left:480px;top:108px;width:108px;height:64px">108 × 64</div>' +
  '<div class="gz" style="left:596px;top:36px;width:36px;height:136px">36</div>' +
  '</div>' +
  '<ol class="rule-list" style="margin-top:22px">' +
  '<li><b>Ein Wert pro Seite.</b> Er steht links in der Fokuskachel, in 48 px. Alles ' +
  'andere ist kleiner.</li>' +
  '<li><b>Rechts wird gehandelt.</b> Module von 64 oder 136 px Höhe, Raster 8 px, ' +
  'Mindestfläche 96 × 64 für alles, was ein Finger trifft.</li>' +
  '<li><b>Das Statusband</b> links an der Fokuskachel trägt den Zustand als Farbe — ' +
  'peripher erkennbar, ohne zu lesen.</li>' +
  '<li><b>Blau ist reserviert.</b> Es heißt: bewegt sich, ist gedrückt, oder ist die ' +
  'Seite, auf der du gerade stehst. Sonst nichts — Zahlen und Zeiten bleiben weiß.</li>' +
  '<li><b>Der Randstreifen</b> trägt die eine sofort wirksame Aktion der Seite. Rot nur, ' +
  'wenn sie etwas anhält.</li>' +
  '</ol></div>' +
  '</div></div>' +

  '<div class="sec"><h2>Bausteine</h2><div class="parts">' +
  '<div class="part"><div class="dev" style="width:244px;height:136px;background:#191F27;' +
  'border-radius:8px;position:relative">' +
  '<div class="band"></div><div class="val">73,5<i>cm</i></div><div class="sub">bereit</div></div>' +
  '<cap>Fokuskachel — Wert, Einheit, Zustand</cap></div>' +

  '<div class="part"><div style="position:relative;width:96px;height:136px">' +
  '<div class="btn" style="position:absolute;left:0;top:0;width:96px;height:64px;background:#191F27">' +
  arrow('up') + '</div>' +
  '<div class="btn on" style="position:absolute;left:0;top:72px;width:96px;height:64px;' +
  'background:#4FA3FF;color:#0E1116">' + arrow('down', '#0E1116') + '</div></div>' +
  '<cap>Haltetaste — Ruhe und gedrückt</cap></div>' +

  '<div class="part"><div style="position:relative;width:108px;height:64px">' +
  '<div class="btn" style="position:absolute;inset:0;background:#191F27">Stehen<small>112,0</small></div>' +
  '</div><cap>Aktionstaste mit Beiwert</cap></div>' +

  '<div class="part"><div style="position:relative;width:118px;height:64px">' +
  '<div class="stat" style="position:absolute;inset:0;background:#191F27"><u>Wind</u><b>11 km/h</b></div>' +
  '</div><cap>Kennwert-Kachel</cap></div>' +

  '<div class="part"><div style="position:relative;width:372px;height:44px">' +
  '<div class="row" style="position:absolute;left:0;top:0;width:372px;background:#191F27">' +
  '<div class="lb">Helligkeit</div><div class="rt"><div class="sl" style="width:150px">' +
  '<div class="tr"></div><div class="fl" style="width:98px"></div>' +
  '<div class="kn" style="left:89px"></div></div></div></div>' +
  '</div><cap>Listenzeile mit Regler</cap></div>' +

  '<div class="part"><div style="position:relative;width:372px;height:42px">' +
  '<div class="lrow" style="position:absolute;left:0;top:0;width:372px;height:42px;background:#191F27">' +
  '<div class="t">11:00</div><div class="n">Review Steuerbox</div><div class="d">45 min</div></div>' +
  '</div><cap>Listenzeile mit Zeit</cap></div>' +

  '<div class="part"><div style="position:relative;width:150px;height:64px">' +
  '<div class="card act" style="position:absolute;inset:0;background:#191F27">' +
  '<div class="cb" style="background:#4FA3FF"></div><n>Schreibtisch</n><v>73,5 cm — bereit</v></div>' +
  '</div><cap>Seitenkachel im Raster</cap></div>' +
  '</div></div>' +

  '<div class="sec"><h2>Symbole — sechs Zeichen, mehr nicht</h2><div class="icons">' +
  icon(wifi('#9AA6B4'), 'wlan') +
  icon('<svg width="16" height="12" viewBox="0 0 16 12" fill="none">' +
    '<path d="M1 3.6C3 1.6 5.4 0.6 8 0.6s5 1 7 3" stroke="#4A535F" stroke-width="1.5" stroke-linecap="round"/>' +
    '<circle cx="8" cy="9.8" r="1.5" fill="#4A535F"/>' +
    '<path d="M2 11 L14 1" stroke="#FFA23D" stroke-width="1.5" stroke-linecap="round"/></svg>', 'wlan-aus') +
  icon(play('#9AA6B4'), 'play') +
  icon(pause('#9AA6B4'), 'pause') +
  icon(next('#9AA6B4'), 'weiter') +
  icon(chev('#9AA6B4'), 'chevron') +
  '</div>' +
  '<div class="note">Pfeil hoch, Pfeil runter, das Stopp-Quadrat und die Seitenpunkte sind ' +
  'reine Geometrie — die zeichnet LVGL selbst, sie brauchen keinen Zeichensatz. „Zurück“ ' +
  'ist „weiter“ gespiegelt. Damit bleiben sechs echte Glyphen.</div></div>';

writeFileSync('design/System.dc.html',
  '<!doctype html>\n<html lang="de">\n<head>\n<meta charset="utf-8">\n' +
  '<script src="./support.js"><\/script>\n</head>\n<body>\n<x-dc>\n<helmet>\n' + FONT +
  '\n<style>' + CSS + SYSCSS + '</style>\n</helmet>\n<div class="sys">' + sysBody +
  '</div>\n</x-dc>\n</body>\n</html>\n');

console.log('geschrieben:', boards.map(b => b.file).join(', '), 'System.dc.html');
