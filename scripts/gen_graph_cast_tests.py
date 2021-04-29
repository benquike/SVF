#!/usr/bin/env python

# /******************************************************************************
# * This program is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# *
# *
# * Author:
# *     Hui Peng <peng124@purdue.edu>
# * Date:
# *     2021-04-26
# *****************************************************************************/

import sys
import yaml
import stringcase


def collect_parents(class_types, parents_store, curr_parent=None):
    for item in class_types:
        if type(item) == str:
            if curr_parent is not None:
                if item not in parents_store:
                    parents_store[item] = []
                parents_store[item].append(curr_parent)
        elif type(item) == dict:
            for k in item.keys():
                if curr_parent is not None:
                    if k not in parents_store:
                        parents_store[k] = []
                    parents_store[k].append(curr_parent)
                collect_parents(item[k], parents_store, k)
        else:
            print(str(item) + ',' + str(type(item)) + ", unexpected type")


def do_parse(yaml_file):
    parents = {}
    abstract_classes = None
    with open(yaml_file) as inf:
        graph_class_info = yaml.full_load(inf)
        abstract_classes = graph_class_info['AbstractClasses']
        # print(graph_class_info)
        info_types = ['NodeTypes', 'EdgeTypes']
        for it in info_types:
            if it in info_types:
                collect_parents(graph_class_info[it], parents)

    # assert('GenericVFGTy' in parents)
    return parents, abstract_classes


def write_alloc_code_for_type(klass, alloc_type, abstract_classes, of, written):
    # skip allocating objects for abstract classes
    if klass in written or klass in abstract_classes:
        return
    written[klass] = True

    if alloc_type == 1:
        # write allocate using new
        of.write("    {} *{} = new {}();\n".
                 format(klass, stringcase.camelcase(klass), klass))

    elif alloc_type == 2:
        # write deallocate using delete
        of.write("    delete {};\n".format(stringcase.camelcase(klass)))

    elif alloc_type == 3:
        # allocate using make_unique
        of.write("    unique_ptr<{}> {} = unique_ptr<{}>();\n".
                 format(klass, stringcase.camelcase(klass), klass))
    elif alloc_type == 4:
        # allocate using make_unique
        of.write("    shared_ptr<{}> {} = make_shared<{}>();\n".
                 format(klass, stringcase.camelcase(klass), klass))


def write_object_allocate_code(parents_info, abstract_classes, of, alloc_type):
    written = {}

    of.write("\n    // BEGIN OF object (de)allocation\n")

    for k in parents_info.keys():
        if k not in written:
            write_alloc_code_for_type(k, alloc_type,
                                      abstract_classes, of, written)
            for v in parents_info[k]:
                write_alloc_code_for_type(v, alloc_type, abstract_classes,
                                          of, written)

    of.write("    // END OF object (de)allocation\n")


def __write_casting_test_for_parent_child(child, parent,
                                          abstract_classes, of,
                                          parents_info, notebook):
    k = (child, parent)

    if k in notebook:
        return

    notebook[k] = True

    # because objects for abstract classes are not created,
    # if the class is an abstract one, skip the test
    if child not in abstract_classes:
        of.write("    ASSERT_TRUE(llvm::isa<{}>({}));\n".
                 format(parent, stringcase.camelcase(child)))
        of.write("    ASSERT_NE(llvm::dyn_cast<{}>({}), nullptr);\n".
                 format(parent, stringcase.camelcase(child)))

    if parent not in abstract_classes:
        of.write("    ASSERT_FALSE(llvm::isa<{}>({}));\n".
                 format(child, stringcase.camelcase(parent)))
        of.write("    ASSERT_EQ(llvm::dyn_cast<{}>({}), nullptr);\n".
                 format(child, stringcase.camelcase(parent)))

    if child in parents_info:
        for p in parents_info[child]:
            __write_casting_test_for_parent_child(child, p,
                                                  abstract_classes, of,
                                                  parents_info, notebook)


def write_normal_casting_test_code(parents_info, abstract_classes, of):
    notebook = {}

    of.write("\n    // BEGIN OF cast testing\n")

    for child in parents_info.keys():
        for parent in parents_info[child]:
            __write_casting_test_for_parent_child(child, parent,
                                                  abstract_classes, of,
                                                  parents_info, notebook)

    of.write("    // END OF cast testing\n\n\n")


def compute_all_parents(parents_info, res, all_classes, child, to):

    if child not in parents_info:
        return

    if to not in res:
        # here we create a new list
        # instead of pointing to the list in parents_info
        res[to] = []

    if child not in all_classes:
        all_classes.append(child)

    res[to].extend(parents_info[child])

    for p in parents_info[child]:
        compute_all_parents(parents_info, res, all_classes, p, to)


def write_abmormal_casting_test_code(all_parents, all_classes,
                                     abstract_classes, of):

    of.write("    // BEGIN of casting unrelated classes\n")
    for c in all_parents.keys():
        for t in all_classes:
            if t not in all_parents[c]:
                of.write("    ASSERT_FALSE(llvm::isa<{}>({}));\n".
                         format(c, stringcase.camelcase(t)))

                of.write("    ASSERT_FALSE(llvm::isa<{}>({}));\n".
                         format(t, stringcase.camelcase(c)))
    of.write("    // END of casting unrelated classes\n\n\n")


def write_prolog(of):
    of.write("/// This file is automatically generated by \n"
             "/// scripts/gen_graph_cast_tests.py graph_type_info.yaml .\n\n"
             "/// DONOT edit this file manually\n\n\n")


def write_gtests_for_casting(parents_info, abstract_classes,
                             all_parents, all_classes):
    with open("casting_test_rawptr.h", "w") as of:
        write_prolog(of)
        write_object_allocate_code(parents_info, abstract_classes, of, 1)
        write_normal_casting_test_code(parents_info, abstract_classes, of)

        # write_abmormal_casting_test_code(all_parents, all_classes,
        #                                 abstract_classes, of)

        # for raw ptrs, deallocate the objects
        write_object_allocate_code(parents_info, abstract_classes, of, 2)

    with open("casting_test_unique_ptr.h", "w") as of:
        write_prolog(of)
        write_object_allocate_code(parents_info, abstract_classes, of, 3)

        write_normal_casting_test_code(parents_info, abstract_classes, of)
        # write_abmormal_casting_test_code(all_parents, all_classes,
        #                                 abstract_classes, of)

    with open("casting_test_shared_ptr.h", "w") as of:
        write_prolog(of)
        write_object_allocate_code(parents_info, abstract_classes, of, 4)
        write_normal_casting_test_code(parents_info, abstract_classes, of)
        # write_abmormal_casting_test_code(all_parents, all_classes,
        #                                 abstract_classes, of)



def main():
    '''
    main function of this script
    '''
    if len(sys.argv) < 2:
        print("please provide the yaml file as the first argument")
        return -1

    parents_info, abstract_classes = do_parse(sys.argv[1])

    all_parents = {}
    all_classes = []
    for c in parents_info.keys():
        compute_all_parents(parents_info, all_parents, all_classes, c, c)

    write_gtests_for_casting(parents_info, abstract_classes,
                             all_parents, all_classes)
    return 0


if __name__ == '__main__':
    sys.exit(main())
