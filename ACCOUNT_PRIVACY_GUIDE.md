# FLauncher Account Privacy Guide

FLauncher can keep each of your Minecraft accounts looking like a separate,
unrelated person. This guide explains every privacy feature, how to set it up,
and where the "off switches" are if something gets in your way.

The core idea: **an observer (a server, another player, an anti-cheat) should
never be able to prove that two of your accounts belong to the same person.**
The things that give that away are your IP address, your username, and your
client fingerprint. FLauncher gives you tools for each.

---

## 1. Lock an instance to one account

**What it does:** an instance is tied to a single account. When you launch it,
that account is used automatically. If that account is missing or broken, the
launch is **refused** — it will never quietly fall back to a different account
(which is how a "main" gets exposed on an alt's instance).

**Set it up:** Edit Instance → Settings → Minecraft tab → check
**"Override Default Account"** and pick the account.

**What you'll see if it's broken:** if the bound account was removed or its
sign-in expired, launching shows a warning and stops. Fix the account (re-add or
re-login) or change the binding, then launch again.

**Escape hatch:** leave "Override Default Account" unchecked for any instance you
want to behave normally (use whatever account you pick at launch).

**Kill switch:** Settings → Launcher → Account Privacy →
**"Lock instances to their assigned account"**. On by default. Turning it off
restores the old behaviour (silently use your default account). Only turn this
off if the lock is getting in your way.

---

## 2. Route an instance through a proxy

**What it does:** gives an instance its own outgoing IP address, so the account
you play on it isn't tied to your real IP.

**Set it up:** Edit Instance → Settings → Minecraft tab →
**"Route This Instance Through a Proxy"**. Fill in:

- **Type:** SOCKS5 (recommended) or HTTP
- **Host / Port:** your proxy's address
- **Username / Password:** only if your proxy requires them
- **Game language:** optional (see section 4)

**How the proxy is used:**

- The launcher passes the proxy to the game's Java process, so the game's own
  HTTP/HTTPS traffic (session checks, Realms, skins) goes through it.
- The launcher also exposes the proxy to mods as an environment variable named
  **`FLAUNCHER_PROXY`** (format: `socks5://user:pass@host:port`). **Your in-game
  proxy mod should read this variable** so the actual server connection is routed
  too. This is the hook that makes your proxy mod "part of the launcher."

**Important — read this:** the in-game *server connection* is routed by your
proxy mod, not by the launcher. The launcher handles the game's web traffic and
hands the proxy to the mod; the mod does the socket-level routing. Make sure your
mod reads `FLAUNCHER_PROXY`.

---

## 3. The pre-launch proxy check (safe-launch gate)

**What it does:** before launching an instance that uses a proxy, FLauncher
quickly checks that the proxy is actually working — that it's reachable and that
it really changes your IP address. If the check fails, you get a warning with a
choice: **Cancel** (default) or **Launch Anyway**.

This is the safety net that stops you from connecting with your real IP because a
proxy silently went down.

**When it runs:** only for instances that use a proxy. Normal instances are never
affected and never slowed down.

**Kill switch:** Settings → Launcher → Account Privacy →
**"Check proxy before launching a proxied instance"**. On by default. Turn it off
to skip the check entirely.

> Note: the check contacts `api.ipify.org` to see your exit IP. It has a short
> timeout, so a slow proxy just means a brief pause, not a hang.

---

## 4. Match your game language to your proxy

**What it does:** Minecraft tells servers what language your client is set to. An
account that always exits through a German proxy but reports US English every
time is a small "tell." You can give each instance its own locale.

**Set it up:** in the same proxy group, set **"Game language"** to something like
`de_DE`, `fr_FR`, `en_GB`, etc. Leave it empty to use the default (English).

This sets the Java locale the game launches with. (The in-game menu language is
still set in Minecraft's own options; this covers the locale the client reports.)

---

## 5. Linkability report

**What it does:** scans all your instances and shows every way two accounts could
be connected, sorted by severity:

- **Critical** — the same account is used by more than one instance.
- **Warning** — two instances share the exact same proxy (same exit IP).
- **For your information** — instances not locked to an account, or with no proxy
  (connecting on your real IP).

**Open it:** Settings → Launcher → Account Privacy → **"Open Linkability
Report…"**. It's read-only; fix the red items first.

---

## 6. Stream-safe mode

**What it does:** hides account usernames (and proxy addresses) in the launcher's
own interface, so you don't leak the account-to-you mapping while streaming or
screen-sharing. Accounts show as "Account 1", "Account 2", etc.

**Turn it on:** Settings → Launcher → Account Privacy → **"Stream-safe mode"**.
Off by default.

---

## Recommended setup for a siloed account

1. Add the account (Accounts → Add Microsoft, or import it).
2. Create an instance for it.
3. Edit Instance → Settings → Minecraft:
   - Override Default Account → pick that account.
   - Route Through a Proxy → set that account's dedicated proxy.
   - Game language → match the proxy's region (optional).
4. Make sure your in-game proxy mod is installed and reads `FLAUNCHER_PROXY`.
5. Launch. The safe-launch gate confirms the proxy is live before the game opens.
6. Run the Linkability Report occasionally to catch mistakes.

---

## All the off switches, in one place

Everything defaults to safe/on. If a feature blocks you, here's how to disable it.
(Settings → Launcher → Account Privacy)

- **Account lock** — "Lock instances to their assigned account"
- **Proxy check** — "Check proxy before launching a proxied instance"
- **Stream-safe** — "Stream-safe mode" (off by default)

Per-instance, uncheck "Override Default Account" or "Route This Instance Through a
Proxy" to make a single instance behave normally.

---

## Honest limitations

- **In-game routing depends on your proxy mod.** The launcher routes the game's
  web traffic and hands off `FLAUNCHER_PROXY`; the actual server connection is the
  mod's job.
- **Cross-launcher Microsoft accounts** (imported from MultiMC/Essential) refresh
  using the client ID they were created under; if Microsoft ever rejects that, the
  account just needs a re-login.
- These features reduce *linkability*; they are not a guarantee. Behaviour, timing,
  and the accounts' own Microsoft/Xbox relationships are outside a launcher's reach.
