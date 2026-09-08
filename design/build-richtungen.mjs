import { writeFileSync } from 'fs';

const GF = '<link rel="stylesheet" href="https://fonts.googleapis.com/css2?' +
  'family=Barlow+Condensed:wght@500;700&family=Space+Grotesk:wght@400;700' +
  '&family=Archivo:wght@500;700&family=Spectral:wght@500;600' +
  '&family=Public+Sans:wght@400;600&family=Montserrat:wght@400;600&display=swap">';

const RESET = `body{margin:0;background:#0B0D10;-webkit-font-smoothing:antialiased}
.scr{position:relative;width:640px;height:180px;overflow:hidden;box-sizing:border-box}`;

function write(file, css, body, root) {
  writeFileSync('design/' + file,
    '<!doctype html>\n<html>\n<head>\n<meta charset="utf-8">\n' +
    '<script src="./support.js"><\/script>\n</head>\n<body>\n<x-dc>\n<helmet>\n' + GF +
    '\n<style>' + RESET + css + '</style>\n</helmet>\n' +
    '<div class="scr ' + root + '">\n' + body + '\n</div>\n</x-dc>\n</body>\n</html>\n');
}

const tri = (d, c, w = 26, h = 16) => d === 'up'
  ? '<svg width="' + w + '" height="' + h + '" viewBox="0 0 26 16"><path d="M13 1 L25 15 L1 15 Z" fill="' + c + '"/></svg>'
  : '<svg width="' + w + '" height="' + h + '" viewBox="0 0 26 16"><path d="M13 15 L1 1 L25 1 Z" fill="' + c + '"/></svg>';

/* ============================================ B — Messinstrument ========= */

const B_CSS = `
.b{background:#14110D;color:#F5EEE0;font-family:Montserrat,system-ui,sans-serif}
.b .top{position:absolute;left:0;top:0;width:640px;height:20px;font-size:12px;color:#8A7F6B}
.b .top .l{position:absolute;left:16px;top:0;line-height:20px}
.b .top .r{position:absolute;right:16px;top:0;line-height:20px;color:#C9BCA3}
.b .hr{position:absolute;left:0;width:640px;height:1px;background:#2C2519}
.b .num{position:absolute;left:20px;top:26px;font-family:'Barlow Condensed',Montserrat,sans-serif;
 font-weight:700;font-size:58px;line-height:58px;letter-spacing:.5px}
.b .num i{font-style:normal;font-size:18px;font-weight:500;color:#8A7F6B;margin-left:6px}
.b .st{position:absolute;right:20px;top:52px;font-size:14px;color:#C9BCA3}
.b .sc{position:absolute;left:0;top:96px;width:640px;height:44px}
.b .tk{position:absolute;bottom:14px;width:1px;background:#4A4132}
.b .tk.mj{background:#6E6250}
.b .lb{position:absolute;top:0;font-size:11px;color:#6E6250;transform:translateX(-50%)}
.b .base{position:absolute;left:24px;bottom:14px;width:592px;height:1px;background:#3A3225}
.b .mk{position:absolute;bottom:6px;width:2px;height:28px;background:#FFB03A}
.b .mkc{position:absolute;bottom:34px;transform:translateX(-50%)}
.b .gh{position:absolute;bottom:14px;width:1px;height:16px;background:#5E5340}
.b .ghl{position:absolute;bottom:0;font-size:11px;color:#8A7F6B;transform:translateX(-50%);white-space:nowrap}
.b .row{position:absolute;left:0;top:138px;width:640px;height:42px;display:flex}
.b .z{position:relative;height:42px;display:flex;align-items:center;justify-content:center;
 font-size:15px;font-weight:600;color:#F5EEE0;border-left:1px solid #2C2519;box-sizing:border-box}
.b .z:first-child{border-left:0}
.b .z small{display:block;font-size:10px;font-weight:400;color:#8A7F6B;margin-top:2px}
.b .z.stop{color:#E2543F}
.b .z.col{flex-direction:column;gap:0}
`;

function bScale(cur, targets) {
  const X0 = 24, X1 = 616, HMIN = 60, HMAX = 125;
  const px = h => X0 + (h - HMIN) * (X1 - X0) / (HMAX - HMIN);
  let out = '<div class="base"></div>';
  for (let h = HMIN; h <= HMAX; h += 5) {
    const mj = h % 10 === 0;
    out += '<div class="tk' + (mj ? ' mj' : '') + '" style="left:' + px(h).toFixed(1) +
      'px;height:' + (mj ? 12 : 6) + 'px"></div>';
    if (mj && h <= 120) out += '<div class="lb" style="left:' + px(h).toFixed(1) + 'px">' + h + '</div>';
  }
  for (const t of targets) {
    out += '<div class="gh" style="left:' + px(t.h).toFixed(1) + 'px"></div>';
    out += '<div class="ghl" style="left:' + px(t.h).toFixed(1) + 'px">' + t.n + '</div>';
  }
  out += '<div class="mk" style="left:' + px(cur).toFixed(1) + 'px"></div>';
  out += '<div class="mkc" style="left:' + px(cur).toFixed(1) + 'px">' + tri('down', '#FFB03A', 12, 8) + '</div>';
  return '<div class="sc">' + out + '</div>';
}

const bRow = zones => '<div class="row">' + zones.map(z =>
  '<div class="z' + (z.c || '') + '" style="width:' + z.w + 'px">' + z.t + '</div>').join('') + '</div>';

write('RichtungB.dc.html', B_CSS,
  '<div class="top"><div class="l">Schreibtisch</div><div class="r">13:45</div></div>' +
  '<div class="hr" style="top:20px"></div>' +
  '<div class="num">73,5<i>cm</i></div><div class="st">bereit</div>' +
  bScale(73.5, [{ h: 73, n: 'Sitzen' }, { h: 112, n: 'Stehen' }]) +
  '<div class="hr" style="top:137px"></div>' +
  bRow([
    { w: 96, t: tri('down', '#F5EEE0') },
    { w: 96, t: tri('up', '#F5EEE0') },
    { w: 96, t: 'Sitzen<small>73,0</small>', c: ' col' },
    { w: 96, t: 'Stehen<small>112,0</small>', c: ' col' },
    { w: 88, t: 'Platz 3<small>Box</small>', c: ' col' },
    { w: 88, t: 'Platz 4<small>Box</small>', c: ' col' },
    { w: 80, t: 'Stopp', c: ' stop' }
  ]), 'b');

write('RichtungBSpotify.dc.html', B_CSS,
  '<div class="top"><div class="l">Spotify — Wohnzimmer</div><div class="r">13:45</div></div>' +
  '<div class="hr" style="top:20px"></div>' +
  '<div class="num" style="font-size:40px;line-height:40px;top:32px">Go Your Own Way</div>' +
  '<div style="position:absolute;left:22px;top:80px;font-size:15px;color:#8A7F6B">Fleetwood Mac</div>' +
  '<div class="sc"><div class="base" style="bottom:16px"></div>' +
  '<div style="position:absolute;left:24px;bottom:16px;width:172px;height:3px;background:#FFB03A"></div>' +
  '<div class="mk" style="left:196px;bottom:8px;height:20px"></div>' +
  '<div class="lb" style="left:34px;top:2px">1:42</div>' +
  '<div class="lb" style="left:604px;top:2px">4:14</div></div>' +
  '<div class="hr" style="top:137px"></div>' +
  bRow([
    { w: 128, t: 'Zurück' }, { w: 128, t: 'Pause' }, { w: 128, t: 'Weiter' },
    { w: 128, t: 'Leiser' }, { w: 128, t: 'Lauter' }
  ]), 'b');

/* ============================================ C — Typografisch =========== */

const C_CSS = `
.c{background:#0A0A0B;color:#FAFAF8;font-family:'Space Grotesk',Montserrat,sans-serif}
.c .top{position:absolute;left:0;top:0;width:640px;height:20px;font-size:12px;color:#63666C}
.c .top .l{position:absolute;left:24px;line-height:20px}
.c .top .r{position:absolute;right:24px;line-height:20px;color:#9DA1A8}
.c .hr{position:absolute;background:#1D1E21}
.c .num{position:absolute;left:24px;top:32px;font-weight:700;font-size:72px;line-height:72px;
 letter-spacing:-2px}
.c .num i{font-style:normal;font-size:20px;font-weight:400;color:#63666C;margin-left:8px;letter-spacing:0}
.c .st{position:absolute;left:26px;top:118px;font-size:15px;color:#9DA1A8}
.c .pre{position:absolute;left:26px;top:150px;font-size:15px;color:#9DA1A8}
.c .pre b{color:#FAFAF8;font-weight:400}
.c .pre s{text-decoration:none;color:#3E4145;margin:0 9px}
.c .act{position:absolute;top:21px;height:159px;display:flex;align-items:center;justify-content:center}
.c .act u{display:block;text-decoration:none;font-size:11px;color:#63666C;position:absolute;bottom:14px;
 left:0;right:0;text-align:center}
.c .fig{position:absolute;text-align:center}
.c .fig b{display:block;font-size:30px;font-weight:700;line-height:34px}
.c .fig u{display:block;text-decoration:none;font-size:11px;color:#63666C;margin-top:2px}
`;

write('RichtungC.dc.html', C_CSS,
  '<div class="top"><div class="l">Schreibtisch</div><div class="r">13:45</div></div>' +
  '<div class="hr" style="left:0;top:20px;width:640px;height:1px"></div>' +
  '<div class="num">73,5<i>cm</i></div>' +
  '<div class="st">bereit</div>' +
  '<div class="hr" style="left:24px;top:140px;width:330px;height:1px"></div>' +
  '<div class="pre"><b>Sitzen</b><s>·</s><b>Stehen</b><s>·</s><b>Platz 3</b><s>·</s><b>Platz 4</b></div>' +
  '<div class="hr" style="left:376px;top:21px;width:1px;height:159px"></div>' +
  '<div class="hr" style="left:508px;top:21px;width:1px;height:159px"></div>' +
  '<div class="act" style="left:377px;width:131px">' + tri('down', '#FAFAF8', 34, 21) + '<u>runter</u></div>' +
  '<div class="act" style="left:509px;width:131px">' + tri('up', '#FAFAF8', 34, 21) + '<u>hoch</u></div>',
  'c');

write('RichtungCWetter.dc.html', C_CSS,
  '<div class="top"><div class="l">Wetter — Berlin</div><div class="r">13:45</div></div>' +
  '<div class="hr" style="left:0;top:20px;width:640px;height:1px"></div>' +
  '<div class="num">18<i>°C</i></div>' +
  '<div class="st" style="font-size:20px;color:#FAFAF8;top:114px">Teils bewölkt</div>' +
  '<div class="hr" style="left:24px;top:146px;width:330px;height:1px"></div>' +
  '<div class="pre" style="top:154px;font-size:13px">gefühlt 17°<s>·</s>62 %<s>·</s>11 km/h<s>·</s>20 % Regen</div>' +
  '<div class="hr" style="left:376px;top:21px;width:1px;height:159px"></div>' +
  '<div class="hr" style="left:508px;top:21px;width:1px;height:159px"></div>' +
  '<div class="fig" style="left:377px;width:131px;top:62px"><b>21°</b><u>Tagesmaximum</u></div>' +
  '<div class="fig" style="left:509px;width:131px;top:62px"><b>12°</b><u>Tagesminimum</u></div>',
  'c');

/* ============================================ D — Zustandsfeld =========== */

const D_CSS = `
.d{font-family:Archivo,Montserrat,sans-serif;color:#FFFFFF;background:#101014}
.d.fahrt{background:#123A63}
.d .top{position:absolute;left:0;top:0;width:640px;height:18px;font-size:11px;
 color:rgba(255,255,255,.45)}
.d .top .l{position:absolute;left:16px;line-height:18px}
.d .top .r{position:absolute;right:16px;line-height:18px;color:rgba(255,255,255,.75)}
.d .split{position:absolute;left:320px;width:1px;background:rgba(255,255,255,.16)}
.d .num{position:absolute;left:0;right:0;top:28px;text-align:center;font-weight:700;
 font-size:58px;line-height:58px;letter-spacing:-1px}
.d .num i{font-style:normal;font-size:18px;font-weight:500;color:rgba(255,255,255,.55);margin-left:6px}
.d .st{position:absolute;left:0;right:0;top:94px;text-align:center;font-size:15px;
 color:rgba(255,255,255,.7)}
.d .arw{position:absolute;top:60px}
.d .arw u{display:block;text-decoration:none;font-size:11px;text-align:center;margin-top:8px;
 color:rgba(255,255,255,.5)}
.d .strip{position:absolute;left:0;top:152px;width:640px;height:28px;display:flex;
 border-top:1px solid rgba(255,255,255,.16)}
.d .strip div{width:160px;height:27px;display:flex;align-items:center;justify-content:center;
 font-size:13px;font-weight:500;color:rgba(255,255,255,.85);border-left:1px solid rgba(255,255,255,.12);
 box-sizing:border-box}
.d .strip div:first-child{border-left:0}
.d .stopbar{position:absolute;left:0;top:150px;width:640px;height:30px;background:#E23E32;
 display:flex;align-items:center;justify-content:center;font-size:14px;font-weight:700;
 letter-spacing:2px}
.d .prog{position:absolute;left:120px;top:124px;width:400px;height:3px;background:rgba(255,255,255,.22)}
.d .prog span{display:block;height:3px;background:#FFFFFF;width:172px}
`;

write('RichtungD.dc.html', D_CSS,
  '<div class="top"><div class="l">Schreibtisch</div><div class="r">13:45</div></div>' +
  '<div class="split" style="top:118px;height:34px"></div>' +
  '<div class="num">73,5<i>cm</i></div><div class="st">bereit</div>' +
  '<div class="arw" style="left:52px">' + tri('down', 'rgba(255,255,255,.9)', 40, 25) + '<u>runter</u></div>' +
  '<div class="arw" style="left:548px">' + tri('up', 'rgba(255,255,255,.9)', 40, 25) + '<u>hoch</u></div>' +
  '<div class="strip"><div>Sitzen</div><div>Stehen</div><div>Platz 3</div><div>Platz 4</div></div>',
  'd');

write('RichtungDFahrt.dc.html', D_CSS,
  '<div class="top"><div class="l">Schreibtisch</div><div class="r">13:45</div></div>' +
  '<div class="num">89,2<i>cm</i></div>' +
  '<div class="st">Ziel 112,0 cm</div>' +
  '<div class="prog"><span></span></div>' +
  '<div class="stopbar">TIPPEN ZUM STOPPEN</div>',
  'd fahrt');

/* ============================================ E — Papier ================= */

const E_CSS = `
.e{background:#E6E2D8;font-family:'Public Sans',Montserrat,sans-serif;color:#1C1A17}
.e .sl{position:absolute;top:8px;height:164px;background:#F7F4EE;border:1px solid #D6D0C3;
 box-sizing:border-box;display:flex;flex-direction:column;align-items:center;justify-content:center;gap:4px}
.e .sl b{font-family:Spectral,serif;font-size:19px;font-weight:600;color:#4A453D}
.e .sl u{text-decoration:none;font-size:9px;color:#8B857A;letter-spacing:.4px}
.e .card{position:absolute;left:60px;top:8px;width:520px;height:164px;background:#FBF9F4;
 border:1px solid #D6D0C3;border-radius:12px;box-sizing:border-box}
.e .hd{position:absolute;left:20px;right:20px;top:12px;height:14px;font-size:11px;color:#8B857A}
.e .hd .r{position:absolute;right:0;top:0}
.e .num{position:absolute;left:20px;top:40px;font-family:Spectral,serif;font-weight:600;
 font-size:54px;line-height:54px;letter-spacing:-1px}
.e .num i{font-style:normal;font-family:'Public Sans',sans-serif;font-size:15px;font-weight:400;
 color:#8B857A;margin-left:6px;letter-spacing:0}
.e .st{position:absolute;left:22px;top:110px;font-size:14px;color:#6B6660}
.e .bt{position:absolute;border:1px solid #CFC8B9;border-radius:8px;background:#FFFFFF;
 display:flex;flex-direction:column;align-items:center;justify-content:center;gap:1px;
 font-size:14px;font-weight:600;color:#1C1A17;box-sizing:border-box}
.e .bt small{font-size:10px;font-weight:400;color:#8B857A}
.e .bt.on{background:#1F5F4B;border-color:#1F5F4B;color:#FBF9F4}
.e .lr{position:absolute;border-top:1px solid #E3DDD0;box-sizing:border-box}
.e .lr t{position:absolute;left:0;top:9px;font-size:14px;font-weight:600;color:#1F5F4B;font-style:normal}
.e .lr n{position:absolute;left:52px;top:9px;font-size:14px;color:#1C1A17;font-style:normal}
.e .lr d{position:absolute;right:0;top:11px;font-size:11px;color:#8B857A;font-style:normal}
`;

write('RichtungE.dc.html', E_CSS,
  '<div class="sl" style="left:-14px;width:66px;border-radius:0 10px 10px 0"><b>18°</b><u>WETTER</u></div>' +
  '<div class="sl" style="right:-14px;width:66px;border-radius:10px 0 0 10px"><b>09:30</b><u>KALENDER</u></div>' +
  '<div class="card">' +
  '<div class="hd">Schreibtisch<span class="r">13:45</span></div>' +
  '<div class="num">73,5<i>cm</i></div><div class="st">bereit</div>' +
  '<div class="bt" style="left:300px;top:42px;width:86px;height:50px">' + tri('up', '#1C1A17', 24, 15) + '</div>' +
  '<div class="bt" style="left:300px;top:100px;width:86px;height:50px">' + tri('down', '#1C1A17', 24, 15) + '</div>' +
  '<div class="bt" style="left:396px;top:42px;width:80px;height:50px">Sitzen<small>73,0</small></div>' +
  '<div class="bt" style="left:480px;top:42px;width:80px;height:50px">Stehen<small>112,0</small></div>' +
  '<div class="bt" style="left:396px;top:100px;width:80px;height:50px">Platz 3<small>Box</small></div>' +
  '<div class="bt" style="left:480px;top:100px;width:80px;height:50px">Platz 4<small>Box</small></div>' +
  '</div>', 'e');

write('RichtungEKalender.dc.html', E_CSS,
  '<div class="sl" style="left:-14px;width:66px;border-radius:0 10px 10px 0"><b>73,5</b><u>TISCH</u></div>' +
  '<div class="sl" style="right:-14px;width:66px;border-radius:10px 0 0 10px"><b>18°</b><u>WETTER</u></div>' +
  '<div class="card">' +
  '<div class="hd">Kalender<span class="r">13:45</span></div>' +
  '<div class="num" style="top:44px">09:30</div>' +
  '<div class="st">Daily Standup · in 12 Minuten</div>' +
  '<div class="lr" style="left:250px;top:34px;width:250px;height:38px;border-top:0">' +
  '<t>11:00</t><n>Review Steuerbox</n><d>45 min</d></div>' +
  '<div class="lr" style="left:250px;top:72px;width:250px;height:38px">' +
  '<t>13:30</t><n>Mittag mit Anna</n><d>1 h</d></div>' +
  '<div class="lr" style="left:250px;top:110px;width:250px;height:38px">' +
  '<t>16:00</t><n>Retro</n><d>30 min</d></div>' +
  '</div>', 'e');

console.log('Richtungen B–E geschrieben (8 Artboards)');
