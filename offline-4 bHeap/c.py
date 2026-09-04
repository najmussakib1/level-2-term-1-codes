"""
Binomial Min-Heap (Part A)

Maintains two binomial min-heaps, H1 and H2, and supports:
    I h x       Insert x into Hh                         O(log n)
    F h         Find Min of Hh                            O(log n)
    E h         Extract Min of Hh                          O(log n)
    D h x y     Decrease key x to y in Hh                  O(log n)*
    R h x       Remove key x from Hh                       O(log n)*
    U h1 h2     Hh1 <- Union(Hh1, Hh2); Hh2 becomes empty  O(log n)
    P h         Print Hh in the fixed checking format

Commands are read from input.txt (one command per line). Every line of
required output (from F, E, and P) is written to output.txt AND printed
to the console, identically.

Design notes:
  - Each binomial tree node stores: key, degree, parent, child (first
    child), sibling (next sibling in the same root/child list).
  - Root lists are kept, at all times, sorted in strictly increasing
    order of degree (the standard binomial-heap invariant), which is
    exactly what CLRS's BINOMIAL-HEAP-UNION produces.
  - Union follows the CLRS merge-then-consolidate algorithm, which
    implements the assignment's convention verbatim:
        * merge root lists by degree (stable: first heap's roots come
          before the second heap's roots on a degree tie)
        * walk the merged list; if the current root and the next root
          have the same degree AND the root after that also has that
          same degree (three in a row), skip linking and just advance
          -- i.e. never link the first two of three equal-degree roots
        * otherwise link the two equal-degree roots, with the smaller
          key becoming the parent and the other root becoming its
          FIRST child (prepended to the child list)
  - Decrease-Key walks key values up toward the root, exchanging key
    values between a node and its parent (never physically relinking
    nodes), exactly as specified.
  - Remove-Key decreases the key below the minimum valid input key
    (-10**9) and then performs Extract-Min, per the assignment's
    sentinel convention (using a sentinel strictly below -10**9, since
    -10**9 itself may already be a valid key in the heap).
  - A global dict maps "current key value" -> node, since keys are
    unique across both heaps at any instant. Decrease-Key updates this
    map every time it swaps a key value between two nodes.
"""

import sys


class Node:
    __slots__ = ("key", "degree", "parent", "child", "sibling")

    def __init__(self, key):
        self.key = key
        self.degree = 0
        self.parent = None
        self.child = None
        self.sibling = None


SENTINEL = -2_000_000_000  # strictly below the minimum valid input key (-10**9)


class BinomialHeap:
    """A single binomial heap: just a pointer to the first root plus a size."""

    def __init__(self):
        self.head = None  # first root; roots linked via .sibling, sorted by degree
        self.size = 0

    def is_empty(self):
        return self.head is None


def _link(y, z):
    """Make y a child of z (z's key is smaller). y becomes z's FIRST child."""
    y.parent = z
    y.sibling = z.child
    z.child = y
    z.degree += 1


def _to_list(head):
    out = []
    while head is not None:
        out.append(head)
        head = head.sibling
    return out


def _merge_root_lists(a_head, b_head):
    """Merge two root lists into one, sorted by non-decreasing degree.
    Stable: on a degree tie, a's root precedes b's root."""
    a = _to_list(a_head)
    b = _to_list(b_head)
    merged = []
    i = j = 0
    while i < len(a) and j < len(b):
        if a[i].degree <= b[j].degree:
            merged.append(a[i]); i += 1
        else:
            merged.append(b[j]); j += 1
    merged.extend(a[i:])
    merged.extend(b[j:])
    for k in range(len(merged) - 1):
        merged[k].sibling = merged[k + 1]
    if merged:
        merged[-1].sibling = None
        return merged[0]
    return None


def _union_heads(a_head, b_head):
    """CLRS BINOMIAL-HEAP-UNION, operating on head pointers; returns new head."""
    head = _merge_root_lists(a_head, b_head)
    if head is None:
        return None

    prev_x = None
    x = head
    next_x = x.sibling

    while next_x is not None:
        if (x.degree != next_x.degree) or (
            next_x.sibling is not None and next_x.sibling.degree == x.degree
        ):
            # different degrees, OR three consecutive roots share this degree:
            # do not link the first two -- just advance.
            prev_x = x
            x = next_x
        else:
            # exactly two consecutive roots share this degree: link them.
            if x.key < next_x.key:
                x.sibling = next_x.sibling
                _link(next_x, x)
            else:
                if prev_x is None:
                    head = next_x
                else:
                    prev_x.sibling = next_x
                _link(x, next_x)
                x = next_x
        next_x = x.sibling

    return head


def union(h1: BinomialHeap, h2: BinomialHeap):
    """Hh1 <- Union(Hh1, Hh2); Hh2 becomes empty."""
    new_head = _union_heads(h1.head, h2.head)
    h1.head = new_head
    h1.size += h2.size
    h2.head = None
    h2.size = 0


def insert(h: BinomialHeap, key, key_map):
    node = Node(key)
    key_map[key] = node
    h.head = _union_heads(node, h.head)
    h.size += 1


def find_min(h: BinomialHeap):
    node = h.head
    best = node.key
    node = node.sibling
    while node is not None:
        if node.key < best:
            best = node.key
        node = node.sibling
    return best


def extract_min(h: BinomialHeap, key_map):
    prev_of_min = None
    p = None
    c = h.head
    min_node = h.head

    while c is not None:
        if c.key < min_node.key:
            min_node = c
            prev_of_min = p
        p = c
        c = c.sibling

    # remove min_node from the root list
    if prev_of_min is None:
        h.head = min_node.sibling
    else:
        prev_of_min.sibling = min_node.sibling

    # reverse min_node's child list (degrees k-1..0) into ascending order (0..k-1)
    rev = None
    child = min_node.child
    while child is not None:
        nxt = child.sibling
        child.sibling = rev
        child.parent = None
        rev = child
        child = nxt

    h.head = _union_heads(h.head, rev)
    h.size -= 1

    del key_map[min_node.key]
    return min_node.key


def decrease_key(node, new_key, key_map):
    old_key = node.key
    del key_map[old_key]
    node.key = new_key
    key_map[new_key] = node

    y = node
    z = y.parent
    while z is not None and y.key < z.key:
        y.key, z.key = z.key, y.key
        key_map[y.key] = y
        key_map[z.key] = z
        y = z
        z = y.parent


def remove_key(h: BinomialHeap, node, key_map):
    decrease_key(node, SENTINEL, key_map)
    extract_min(h, key_map)


def print_heap(h_id, h: BinomialHeap, out_lines):
    out_lines.append(f"Printing Binomial Heap H{h_id}")
    out_lines.append(f"Heap size: {h.size}")
    if h.size == 0:
        out_lines.append(f"Heap H{h_id} is empty.")
        return

    node = h.head
    while node is not None:
        out_lines.append(f"Binomial Tree, B{node.degree}")
        level_nodes = [node]
        level = 0
        while level_nodes:
            keys = sorted(n.key for n in level_nodes)
            out_lines.append(f"Level {level}: " + " ".join(str(k) for k in keys))
            next_level = []
            for n in level_nodes:
                c = n.child
                while c is not None:
                    next_level.append(c)
                    c = c.sibling
            level_nodes = next_level
            level += 1
        node = node.sibling


def main():
    with open("input.txt", "r") as f:
        lines = [ln.strip() for ln in f if ln.strip() != ""]

    heaps = {1: BinomialHeap(), 2: BinomialHeap()}
    key_map = {}

    out_lines = []

    for line in lines:
        parts = line.split()
        cmd = parts[0]

        if cmd == "I":
            h = int(parts[1]); x = int(parts[2])
            insert(heaps[h], x, key_map)

        elif cmd == "F":
            h = int(parts[1])
            out_lines.append(f"Find Min returned: {find_min(heaps[h])}")

        elif cmd == "E":
            h = int(parts[1])
            result = extract_min(heaps[h], key_map)
            out_lines.append(f"Extract Min returned: {result}")

        elif cmd == "D":
            h = int(parts[1]); x = int(parts[2]); y = int(parts[3])
            node = key_map[x]
            decrease_key(node, y, key_map)

        elif cmd == "R":
            h = int(parts[1]); x = int(parts[2])
            node = key_map[x]
            remove_key(heaps[h], node, key_map)

        elif cmd == "U":
            h1 = int(parts[1]); h2 = int(parts[2])
            union(heaps[h1], heaps[h2])

        elif cmd == "P":
            h = int(parts[1])
            print_heap(h, heaps[h], out_lines)

        else:
            raise ValueError(f"Unrecognized command: {line}")

    output_text = "\n".join(out_lines)
    with open("output.txt", "w") as f:
        f.write(output_text + ("\n" if out_lines else ""))

    sys.stdout.write(output_text + ("\n" if out_lines else ""))


if __name__ == "__main__":
    main()