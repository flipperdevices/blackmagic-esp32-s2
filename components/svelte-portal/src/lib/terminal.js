const escSeq = {
    "7": null,
    "8": null,
    "[20h": null,
    "[?1h": null,
    "[?3h": null,
    "[?4h": null,
    "[?5h": null,
    "[?6h": null,
    "[?7h": null,
    "[?8h": null,
    "[?9h": null,
    "[20l": null,
    "[?1l": null,
    "[?2l": null,
    "[?3l": null,
    "[?4l": null,
    "[?5l": null,
    "[?6l": null,
    "[?7l": null,
    "[?8l": null,
    "[?9l": null,
    "=": null,
    ">": null,
    "(A": null,
    ")A": null,
    "(B": null,
    ")B": null,
    "(0": null,
    ")0": null,
    "(1": null,
    ")1": null,
    "(2": null,
    ")2": null,
    "N": null,
    "O": null,
    //   "[m": function (state) { if (state.spanCount > 0) {state.output +=
    // '</span>'; state.spanCount--;} },   "[0m": function (state) { if
    // (state.spanCount > 0) {state.output += '</span>'; state.spanCount--;} },
    // "[1m": { 'class': 'bold' },   "[2m": { 'class': 'light' },   "[4m": {
    // 'class': 'underline' },   "[5m": { 'class': 'blink' },   "[7m": { 'class':
    // 'reverse' },   "[8m": { 'class': 'invisible' },
    "[;r": null,
    "[A": null,
    "[B": null,
    "[C": null,
    "[D": null,
    "[H": null,
    "[;H": null,
    "[f": null,
    "[;f": null,
    "D": null,
    "M": null,
    "E": null,
    "H": null,
    "[g": null,
    "[0g": null,
    "[3g": null,
    "#3": null,
    "#4": null,
    "#5": null,
    "#6": null,
    "[K": null,
    "[0K": null,
    "[1K": null,
    "[2K": null,
    "[J": null,
    "[0J": null,
    "[1J": null,
    "[2J": null,
    "5n": null,
    "0n": null,
    "3n": null,
    "6n": null,
    ";R": null,
    "[c": null,
    "[0c": null,
    "[?1;0c": null,
    "c": null,
    "#8": null,
    "[2;1y": null,
    "[2;2y": null,
    "[2;9y": null,
    "[2;10y": null,
    "[0q": null,
    "[1q": null,
    "[2q": null,
    "[3q": null,
    "[4q": null
}

const modeClasses = {
    '1': 'bold',
    '2': 'light',
    '3': 'underline',
    '4': 'blink',
    '5': 'reverse',
    '6': 'invisible'
}

const modeStyles = {

    '30': 'color: black',
    '31': 'color: red',
    '32': 'color: green',
    '33': 'color: yellow',
    '34': 'color: blue',
    '35': 'color: magenta',
    '36': 'color: cyan',
    '37': 'color: white',

    '40': 'background-color: black',
    '41': 'background-color: red',
    '42': 'background-color: green',
    '43': 'background-color: yellow',
    '44': 'background-color: blue',
    '45': 'background-color: magenta',
    '46': 'background-color: cyan',
    '47': 'background-color: white'
}

function processModes(escapeTxt, state) {
    var modes = escapeTxt.substring(1, escapeTxt.length - 1);

    if (modes.length > 0) {
        modes = modes.split(';');
        for (let i = 0; i < modes.length; i++) {
            if (modeClasses[modes[i]]) {
                state
                    .classes
                    .push(modeClasses[modes[i]]);
            } else if (modeStyles[modes[i]]) {
                state
                    .styles
                    .push(modeStyles[modes[i]]);
            } else if (modes[i] === '0') {
                if (state.spanCount > 0) {
                    state.output += '</span>';
                    state.spanCount--;
                }
            }
        }
    } else {
        if (state.spanCount > 0) {
            state.output += '</span>';
            state.spanCount--;
        }
    }
}

function isLetter(str) {
    return str.length === 1 && str.match(/[a-z]/i);
}

function isDigit(str) {
    return str.length === 1 && str.match(/[0-9]/i);
}

function processEscape(escapeTxt, state) {
    if (escapeTxt.startsWith('[') && escapeTxt.endsWith('m')) {
        processModes(escapeTxt, state);
    } else {
        const entry = escSeq[escapeTxt];
        if (entry && entry !== null) {
            if (typeof entry === 'object') {
                if (entry.class) {
                    state
                        .classes
                        .push(entry.class);
                }
                if (entry.style) {
                    state
                        .styles
                        .push(entry.stye);
                }
            } else if (typeof entry === 'function') {
                entry(state);
            }
        }
    }
}

export default function parseTerminal(text) {

    var escapeTxt = '';

    var state = {
        output: '',
        spanCount: 0,
        classes: [],
        styles: []
    }

    for (let i = 0; i < text.length; i++) {
        let character = text.charAt(i);

        if (character === '\u001b') {
            escapeTxt = text.charAt(++i);
            if (escapeTxt === '[') {
                // process until character
                do {
                    character = text.charAt(++i)
                    escapeTxt += character;
                } while (!isLetter(character) && i < text.length);
            } else if (escapeTxt === '#') {
                // process until digit
                do {
                    character = text.charAt(++i)
                    escapeTxt += character;
                } while (!isDigit(character) && i < text.length);
            } else if (escapeTxt === '(' || escapeTxt === ')') {
                // process another char
                escapeTxt += text.charAt(++i);
            } else {
                // that's the escape
            }

            processEscape(escapeTxt, state);

        } else {
            if (state.classes.length > 0 || state.styles.length > 0) {
                state.output += `<span 
                    class="${state.classes.join(' ')}"
                    style="${state.styles.join(';')}
                ">`;
                state.classes = [];
                state.styles = [];
                state.spanCount++;
            }

            if (character === ' ') {
                state.output += '&nbsp;';
            } else {
                state.output += character;
            }
        }
    }

    // replace single &nbsp; enclosed with non &nbsp; characters with spaces
    state.output = state
        .output
        .replace(/&nbsp;([^&]+)&nbsp;/g, ' $1 ');

    // return first space to &nbsp;
    if (state.output.startsWith(' ')) {
        state.output = '&nbsp;' + state.output.substring(1);
    }


    for (let i = 0; i < state.spanCount; i++) {
        state.output += '</span>';
    }

    return state.output;
}
