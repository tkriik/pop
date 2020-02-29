#!/usr/bin/env python3

# Term types
INTEGER = 'INTEGER'
WORD = 'WORD'
BUILTIN = 'BUILTIN'
ADDRESS = 'ADDRESS'
RETURN = 'RETURN'

BUILTINS = [
        ('.', False),
        ('.s', False),
        ('+', False),
        ('def', True),
        ('end', True)
]

class Term:
    def __init__(self, type, value, immediate = False):
        self.type = type
        self.value = value
        self.immediate = immediate

    def __str__(self):
        if self.type == INTEGER:
            return 'integer<{}>'.format(self.value)
        elif self.type == WORD:
            return 'word<{}>'.format(repr(self.value))
        elif self.type == BUILTIN:
            return 'builtin<{}, immediate = {}>'.format(repr(self.value), self.immediate)
        elif self.type == ADDRESS:
            return 'address<{}>'.format(self.value)
        elif self.type == RETURN:
            return 'return'
        else:
            return 'INVALID<{}, {}, immediate = {}'.format(self.type, repr(self.value), self.immediate)

    def is_literal(self):
        return self.type == INTEGER

class Scanner:
    def __init__(self):
        self.data = ''
        self.pos = 0

    def refresh(self):
        self.data = input('>> ') + '\n'
        self.pos = 0

    def read_char(self):
        if self.pos >= len(self.data):
            try:
                self.refresh()
            except:
                return None
        c = self.data[self.pos]
        self.pos += 1
        return c

    def read_word(self):
        w = ''
        while True:
            c = self.read_char()
            if c is None:
                return None
            if c.isspace():
                continue
            else:
                w += c
                break
        while True:
            c = self.read_char()
            if c is None or c.isspace():
                return w
            w += c

    def read_term(self):
        w = self.read_word()
        if w is None:
            return None
        try:
            i = int(w)
            return Term(INTEGER, i)
        except ValueError:
            pass
        return Term(WORD, w)

class VM:
    def __init__(self):
        self.scanner = Scanner()

        self.stack = []
        self.rstack = []

        self.env = {}
        for (builtin, immediate) in BUILTINS:
            self.env[builtin] = Term(BUILTIN, builtin, immediate = immediate)

        self.mode = 'execute'

        self.pc = 0
        self.program = []

    def dump(self):
        print('----- BEGIN DEBUG -----')
        print('scanner.data = {}'.format(repr(self.scanner.data)))
        print('scanner.pos = {}'.format(self.scanner.pos))
        print('stack = {}'.format(repr(self.stack)))
        print('env = {{{}}}'.format(', '.join('\'{}\' => {}'.format(k, v) for k, v in self.env.items())))
        print('pc = {}'.format(self.pc))
        print('program = [{}]'.format(', '.join(str(x) for x in self.program)))
        print('----- END DEBUG -----')

    def run(self):
        while True:
            self.dump()

            if self.pc < len(self.program):
                term = self.program[self.pc]
                self.pc += 1
            else:
                term = self.scanner.read_term()
                if term is None:
                    return

            if self.mode == 'execute' and term.is_literal():
                self.stack.append(term.value)
                continue

            res = term if term.type is not WORD else self.env.get(term.value)
            if res is None:
                print('undefined word: {}'.format(term.value))
                continue

            if self.mode == 'compile' and not res.immediate:
                self.program.append(res)
                self.pc += 1
                continue

            if res.value == '.':
                try:
                    x = self.stack.pop()
                    print(x)
                except IndexError:
                    print('stack underflow')
                    continue

            elif res.value == '.s':
                print('<{}>'.format(len(self.stack)), end = '')
                for x in self.stack:
                    print(' {}'.format(x), end = '')
                print('')

            elif res.value == '+':
                try:
                    a = self.stack.pop()
                    b = self.stack.pop()
                    self.stack.append(a + b)
                except IndexError:
                    print('stack underflow')
                    continue

            elif res.value == 'def':
                word = self.scanner.read_term()
                if word is None:
                    break
                if word.is_literal():
                    print('cannot define literal: {}'.format(word.value))
                    break
                self.env[word.value] = Term(ADDRESS, self.pc)
                self.mode = 'compile'

            elif res.value == 'end':
                self.program.append(Term(RETURN, 'return'))
                self.mode = 'execute'
                self.pc += 1

            elif res.type == ADDRESS:
                self.rstack.append(self.pc)
                self.pc = res.value

            elif res.type == RETURN:
                try:
                    self.pc = self.rstack.pop()
                except IndexError:
                    print('return stack underflow')
                    break

def main():
    vm = VM()
    vm.run()
    #scanner = Scanner()
    #while True:
    #    t = scanner.read_token()
    #    if t is None:
    #        print('EOF')
    #        break;
    #    print('word: "{}"'.format(t))

main()
