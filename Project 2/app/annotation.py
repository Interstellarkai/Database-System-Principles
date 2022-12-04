from collections import deque


def get_plan_summary(plan):
    """
    Produces a summary given a query plan.
    - Total cost of all nodes
    - Total number of nodes
    """

    summary = {
        "total_cost": 0,
        "nodes_count": 0,
    }

    # queue for bfs
    q = deque([plan])
    while q:
        n = len(q)
        for _ in range(n):
            node = q.popleft()
            summary["nodes_count"] += 1
            summary["total_cost"] += node["Total Cost"]
            if "Plans" in node:
                for child in node["Plans"]:
                    q.append(child)

    return summary


bold = {"START": "<b>", "END": "</b>"}


def bold_string(string):
    return bold["START"] + string + bold["END"]


def aggregate_natural_explain(plan):
    strategy = plan["Strategy"]
    if strategy == "Sorted":
        result = f"{bold_string('Aggregate')} operation is perfomed on the rows based on their keys."
        if "Group Key" in plan:
            result += f" The {bold_string('aggregated')} key(s) are: "
            for key in plan["Group Key"]:
                result += bold_string(key) + ","
            result = result[:-1] + "."
        if "Filter" in plan:
            result += " The rows are also filtered by " + bold_string(
                plan["Filter"].replace("::text", "")
            )
            result += "."
        return result
    elif strategy == "Hashed":
        result = f"{bold_string('Aggregate')} operation is performed by hashing on all rows based on the following key(s): "
        for key in plan["Group Key"]:
            result += bold_string(key.replace("::text", "")) + ", "
        result += f"then the results are {bold_string('aggregated')} into bucket(s) accordung to the hashed key."
        return result
    elif strategy == "Plain":
        return (
            f"Normal {bold_string('Aggregate')} operation is performed on the result."
        )
    else:
        return "Aggregation is performed."


def append_natural_explain():
    result = f"{bold_string('Append')} operation is performed with multiple sub-operations. All the rows that are returned as one set as the result."
    return result


def cte_natural_explain(plan):
    result = (
        f"A {bold_string('CTE scan')} operation is performed on the table "
        + bold_string(str(plan["CTE Name"]))
        + " which is stored in memory "
    )
    if "Index Cond" in plan:
        result += " with the condition(s) " + bold_string(
            plan["Index Cond"].replace("::text", "")
        )
    if "Filter" in plan:
        result += " the result is then filtered by " + bold_string(
            plan["Filter"].replace("::text", "")
        )
    result += "."
    return result


def function_scan_natural_explain(plan):
    return "The function {} is run and returns all the recordset(s) that it created.".format(
        bold_string(plan["Function Name"])
    )


def group_natural_explain(plan):
    result = f"The result from the previous operation is {bold_string('grouped')} by the following key(s): "
    for i, key in enumerate(plan["Group Key"]):
        result += bold_string(key.replace("::text", ""))
        if i == len(plan["Group Key"]) - 1:
            result += "."
        else:
            result += ", "
    return result


def index_scan_natural_explain(plan):
    result = ""
    result += (
        f"{bold_string('Index Scan')} operation is performed using "
        + bold_string(plan["Index Name"])
        + " index table "
    )
    if "Index Cond" in plan:
        result += " with the following condition(s): " + bold_string(
            plan["Index Cond"].replace("::text", "")
        )
    result += ", and the {} table and fetches rows that matches the conditions.".format(
        bold_string(plan["Relation Name"])
    )

    if "Filter" in plan:
        result += (
            " The result is then filtered by "
            + bold_string(plan["Filter"].replace("::text", ""))
            + "."
        )
    return result


def index_only_scan_natural_explain(plan):
    result = ""
    result += (
        f"An {bold_string('Index Scan')} operation is done using "
        + bold_string(plan["Index Name"])
        + " index table"
    )
    if "Index Cond" in plan:
        result += " with the condition(s) " + bold_string(
            plan["Index Cond"].replace("::text", "")
        )
    result += ". Matches are then returned as the result."
    if "Filter" in plan:
        result += (
            " The result is finally filtered by: "
            + bold_string(plan["Filter"].replace("::text", ""))
            + "."
        )

    return result


def limit_natural_explain(plan):
    result = f"A scan is performed with a {bold_string('limit')} of {plan['Plan Rows']} entries."
    return result


def materialize_natural_explain():
    result = f"{bold_string('Materialize')} operation is performed. This means the results of previous operation(s) are stored in physical memory/disk for faster access."
    return result


def unique_natural_explain():
    result = f"A scan is performed on previous results to remove {bold_string('un-unique')} values."
    return result


def merge_join_natural_explain(plan):
    result = f"{bold_string('Merge Join')} operation is performed on results from sub-operations"

    if "Merge Cond" in plan:
        result += " on the condition " + bold_string(
            plan["Merge Cond"].replace("::text", "")
        )

    if "Join Type" == "Semi":
        result += " but only the rows from the left relation is returned as the result"

    result += "."
    return result


def setop_natural_explain(plan):
    result = "Results are returned base on the"
    cmd_name = bold_string(str(plan["Command"]))
    if cmd_name == "Except" or cmd_name == "Except All":
        result += "differences "
    else:
        result += "similarities "
    result += (
        "between the two previously scanned tables using the {} operation.".format(
            bold_string(plan["Node Type"])
        )
    )

    return result


def subquery_scan_natural_explain():
    result = f"{bold_string('Subquery scan')} operation is performed on results from sub-operations without any changes."
    return result


def values_scan_natural_explain():
    result = f"A {bold_string('Values Scan')} operation is performed using the values given in query."
    return result


def seq_scan_natural_explain(plan):
    sentence = f"{bold_string('Sequential Scan')} operation is performed on relation "
    if "Relation Name" in plan:
        sentence += bold_string(plan["Relation Name"])
    if "Alias" in plan:
        if plan["Relation Name"] != plan["Alias"]:
            sentence += " with the alias of {}".format(plan["Alias"])
    if "Filter" in plan:
        sentence += " and filtered by {}".format(plan["Filter"].replace("::text", ""))
    sentence += "."

    return sentence


def nested_loop_natural_explain():
    result = f"{bold_string('Nested Loop')} is performed to join results between the scans of the suboperations."
    return result


def sort_natural_explain(plan):
    result = f"The result is {bold_string('Sorted')} using the attribute "
    if "DESC" in plan["Sort Key"]:
        result += (
            bold_string(str(plan["Sort Key"].replace("DESC", "")))
            + " in descending order of "
        )
    elif "INC" in plan["Sort Key"]:
        result += (
            bold_string(str(plan["Sort Key"].replace("INC", "")))
            + " in ascending order of "
        )
    else:
        result += bold_string(str(plan["Sort Key"]))
    result += "."
    return result


def hash_natural_explain():
    result = f"{bold_string('Hash')} function is used to make a memory {bold_string('hash')} using the table rows."
    return result


def hash_join_natural_explain(plan):
    result = f"The result from previous operation is joined using {bold_string('Hash')} {bold_string(plan['Join Type'])} {bold_string('Join')}"
    if "Hash Cond" in plan:
        result += " on the condition: {}".format(
            bold_string(plan["Hash Cond"].replace("::text", ""))
        )
    result += "."
    return result


def bitmap_heap_scan_natural_explain(plan):
    result = f"With the result from the previous {bold_string('Bitmap Index Scan')}, {bold_string('Bitmap Heap Scan')} is performed on {bold_string(plan['Relation Name'])} table. To get results matching the condition {bold_string(plan['Recheck Cond'])} which is used to create the Bitmap."
    return result


def bitmap_index_scan_natural_explain(plan):
    result = f"{bold_string('Bitmap Index Scan')} is performed on {bold_string(plan['Index Name'])} with index condition of {bold_string(plan['Index Cond'])} to create a Bitmap."
    return result


def memoize_natural_explain(plan):
    result = f"The previous sub-operation result is then {bold_string('Memoized')}. This means that the result is cached with cache key of {bold_string(plan['Cache Key'])}."
    return result


def gather_merge_natural_explain():
    result = f"{bold_string('Gather Merge')} operation is performed on the results from parallel sub-operations. The results {bold_string('Sorted')} order is preserved."
    return result


def gather_natural_explain():
    result = f"{bold_string('Gather')} operation is performed on the results from parallel sub-operations. The results order is {bold_string('Not')} preserved unlike {bold_string('Gather Merge')}."
    return result


def natural_explain(plan):
    match plan["Node Type"]:
        case "Aggregate":
            return aggregate_natural_explain(plan)
        case "Append":
            return append_natural_explain()
        case "CTE Scan":
            return cte_natural_explain(plan)
        case "Function Scan":
            return function_scan_natural_explain(plan)
        case "Group":
            return group_natural_explain(plan)
        case "Index Scan":
            return index_scan_natural_explain(plan)
        case "Index Only Scan":
            return index_only_scan_natural_explain(plan)
        case "Limit":
            return limit_natural_explain(plan)
        case "Materialize":
            return materialize_natural_explain()
        case "Unique":
            return unique_natural_explain()
        case "Merge Join":
            return merge_join_natural_explain(plan)
        case "SetOp":
            return setop_natural_explain(plan)
        case "Subquery Scan":
            return subquery_scan_natural_explain()
        case "Values Scan":
            return values_scan_natural_explain()
        case "Seq Scan":
            return seq_scan_natural_explain(plan)
        case "Nested Loop":
            return nested_loop_natural_explain()
        case "Sort":
            return sort_natural_explain(plan)
        case "Hash":
            return hash_natural_explain()
        case "Hash Join":
            return hash_join_natural_explain(plan)
        case "Bitmap Heap Scan":
            return bitmap_heap_scan_natural_explain(plan)
        case "Bitmap Index Scan":
            return bitmap_index_scan_natural_explain(plan)
        case "Memoize":
            return memoize_natural_explain(plan)
        case "Gather Merge":
            return gather_merge_natural_explain()
        case "Gather":
            return gather_natural_explain()
        case _:
            return plan["Node Type"]
