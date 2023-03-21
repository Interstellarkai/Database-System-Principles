from collections import deque

import psycopg2
from psycopg2 import ProgrammingError, OperationalError

from annotation import get_plan_summary, natural_explain

# mapping of node type to runtime setting name
params_map = {
    "bitmapscan": "enable_bitmapscan",
    "hashagg": "enable_hashagg",
    "Hash Join": "enable_hashjoin",
    "Index Scan": "enable_indexscan",
    "Index Only Scan": "enable_indexonlyscan",
    "Materialize": "enable_material",
    "Merge Join": "enable_mergejoin",
    "Nested Loop": "enable_nestloop",
    "Seq Scan": "enable_seqscan",
    "Sort": "enable_sort",
    "Tid Scan": "enable_tidscan",
}


class DatabaseConnection:
    """
    Database connection singleton class.
    Call get_conn() to get a database connection.
    """

    _conn = None

    @classmethod
    def get_conn(cls):
        if cls._conn is None:
            cls._conn = psycopg2.connect(
                host="postgres",
                database="TPC-H",
                user="postgres",
                password="password123",
                port="5432",
            )
        return cls._conn


def get_node_types(plan):
    # list to store all node types
    types = []

    # queue for bfs
    q = deque([plan])
    while q:
        n = len(q)
        for _ in range(n):
            node = q.popleft()
            types.append(node["Node Type"])
            if "Plans" in node:
                for child in node["Plans"]:
                    q.append(child)

    return types


def get_plan_comparison_attr(plan):
    inter_result = {"text": {"name": ""}}

    # get all key:value pair with string value
    for value in plan.items():
        if isinstance(value, str):
            inter_result["text"]["name"] += value + " "

    # remove last space from string
    inter_result["text"]["name"] = inter_result["text"]["name"][:-1]

    children_details = []

    if plan.get("Plans") is not None:
        for plan in plan.get("Plans"):
            children_details.append(get_plan_comparison_attr(plan))

    if len(children_details) != 0:
        inter_result["children"] = children_details

    return inter_result


def get_natural_explanation(plan):
    natural_explanation = []

    # queue for bfs
    q = deque([plan])
    while q:
        n = len(q)
        for _ in range(n):
            node = q.popleft()
            natural_explanation.append(natural_explain(node))
            if "Plans" in node:
                for child in node["Plans"]:
                    q.append(child)

    return natural_explanation[::-1]


def get_plans(user_query):
    """
    Get query plans for the given user query.

    Returns:
        - error boolean: True if error, False otherwise
        - result dictionary: contains message if error, else contains plan_data, summary_data and natural_explain
    """

    # query string with placeholder
    query_str = f"EXPLAIN (ANALYZE false, SETTINGS true, FORMAT JSON) {user_query};"

    # dictionary to store results
    result = {
        "plan_data": [],
        "summary_data": [],
        "natural_explain": [],
    }

    try:
        # get database connection
        conn = DatabaseConnection.get_conn()

        # get base query plan
        with conn.cursor() as cur:
            try:
                # execute query and get results
                cur.execute(query_str)
                plan1 = cur.fetchall()[0][0][0].get("Plan")
            except ProgrammingError as e:
                print(e)
                cur.execute("ROLLBACK;")
                return True, {"msg": f"Invalid SQL query!"}

        # get alternative plans by passing in the previous plan
        plan2 = get_alt_plan(query_str, prev_plan=plan1)
        plan3 = get_alt_plan(query_str, prev_plan=plan2)

        # parse results for summary for plan 1
        summary1 = get_plan_summary(plan1)

        # get comparison attributes for plans
        plan_comp_attr1 = get_plan_comparison_attr(plan1)
        plan_comp_attr2 = get_plan_comparison_attr(plan2)
        plan_comp_attr3 = get_plan_comparison_attr(plan3)

        # get natural explanation for plan 1
        natural_explain1 = get_natural_explanation(plan1)

        # add plan 1 results to result object
        result["plan_data"].append(plan1)
        result["summary_data"].append(summary1)
        result["natural_explain"].append(natural_explain1)

        # check if plan2 is the same as plan1
        if plan_comp_attr2 != plan_comp_attr1:
            result["plan_data"].append(plan2)

            # parse results for summary for plan 2
            summary2 = get_plan_summary(plan2)
            result["summary_data"].append(summary2)

            # get natural explanation for plan 2
            natural_explain2 = get_natural_explanation(plan2)
            result["natural_explain"].append(natural_explain2)

        # check if plan3 is the same as plan1 or plan2
        if plan_comp_attr3 != plan_comp_attr1 and plan_comp_attr3 != plan_comp_attr2:
            result["plan_data"].append(plan3)

            # parse results for summary for plan 3
            summary3 = get_plan_summary(plan3)
            result["summary_data"].append(summary3)

            # get natural explanation for plan 3
            natural_explain3 = get_natural_explanation(plan3)
            result["natural_explain"].append(natural_explain3)

    except OperationalError as e:
        return True, {
            "msg": f"An error has occurred: Failed to connect to the database! Please ensure that the database is running."
        }
    except Exception as e:
        return True, {"msg": f"An error has occurred: {repr(e)}"}

    # return False for no error, and results
    return False, result


def get_alt_plan(query_str, prev_plan):
    # Get all node type from prev_plan
    node_types = get_node_types(prev_plan)

    # Get list of query config settings parameters
    query_config_params = []
    for nt in node_types:
        if nt in params_map:
            query_config_params.append(params_map.get(nt))

    # get database connection
    conn = DatabaseConnection.get_conn()

    with conn.cursor() as cur:
        try:
            # Reset query plan configuration at the start
            cur.execute("RESET ALL;")

            # turn off each setting
            for item in query_config_params:
                cur.execute("SET LOCAL {} TO off;".format(item))

            # execute query and get results
            cur.execute(query_str)
            alt_plan = cur.fetchall()[0][0][0].get("Plan")

            # Reset query plan configuration at the end
            cur.execute("RESET ALL;")

        except Exception as e:
            # manage exceptions by rolling back transaction
            cur.execute("ROLLBACK;")
            # then re-raise exception from calling function to handle
            raise e

    return alt_plan
